#include <FrameDecoder.hpp>

FrameDecoder* FrameDecoder::pinstance_{nullptr};
std::mutex FrameDecoder::mutex_;

FrameDecoder *FrameDecoder::GetInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pinstance_ == nullptr)
    {
        pinstance_ = new FrameDecoder();
    }
    return pinstance_;
}

FrameDecoder::FrameDecoder()
{
    int ret = prepareDecoder();
    if (ret < 0)
    {
        std::cerr << "Failed to prepare decoder! " << std::endl;
        exit(-1);
    }

    std::cout << "Starting raw packet decoding thread" << std::endl;
    data_processing_thread = std::thread(&FrameDecoder::processRawPacketData, this);
}

FrameDecoder::~FrameDecoder()
{
    cleanUp();
}

void FrameDecoder::cleanUp()
{
    stop_processing = true;

    queue_cv.notify_one();

    if (data_processing_thread.joinable())
    {
        data_processing_thread.join();
    }

    printf("Cleaning up frame decoder...\n");
}

int FrameDecoder::prepareDecoder()
{
    std::cout << "Preparing decoder..." << std::endl;
    av_log_set_level(AV_LOG_DEBUG);

    // Allocate memory for decoder
    decoder = new StreamingContext;

    // Find video codec
    decoder->video_codec = avcodec_find_decoder(AV_CODEC_ID_MPEG4);
    if (!decoder->video_codec) {
        std::cerr << "Codec not found" << std::endl;
        exit(1);
    }

    // Allocate codec context
    decoder->video_codec_context = avcodec_alloc_context3(decoder->video_codec);
    if (!decoder->video_codec_context) {
        std::cerr << "Could not allocate video codec context" << std::endl;
        exit(1);
    }

    // Open codec
    if (avcodec_open2(decoder->video_codec_context, decoder->video_codec, NULL) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        exit(1);
    }

    // Initialize parser
    parser = av_parser_init(decoder->video_codec->id);
    if (!parser) {
        std::cerr << "Parser not found" << std::endl;
        exit(1);
    }

    std::cout << "Decoder prepared!" << std::endl;
    return 0;
}


void FrameDecoder::processRawPacketData()
{
    while(!stop_processing)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mtx);

            queue_cv.wait(lock, [&]() {
                return !rawDataQueue.empty() || stop_processing;
            });
        }

        if(stop_processing && rawDataQueue.empty())
            break;

        ReconstructedPacket* rpkt = getRawPacketFromQueue();
        if(rpkt)
        {
            rawDataToFrames(rpkt);
        }
    }
}

ReconstructedPacket* FrameDecoder::getRawPacketFromQueue()
{
    if(rawDataQueue.empty())
        return nullptr;

    ReconstructedPacket* rawPacket = rawDataQueue.front();
    rawDataQueue.pop();
    return rawPacket;
}

void FrameDecoder::addRawDataToQueue(ReconstructedPacket* rpkt)
{
    {
        std::lock_guard<std::mutex> lock(queue_mtx);
        rawDataQueue.push(rpkt);
    }
    queue_cv.notify_one();
}

void FrameDecoder::addFrameToQueue(AVFrame* frame)
{
    {
        std::lock_guard<std::mutex> lock(frame_queue_mtx);
        decodedFramesQueue.push(frame);
    }
}

AVFrame* FrameDecoder::getFrameFromQueue()
{
    if(decodedFramesQueue.empty())
        return nullptr;

    AVFrame* frame = decodedFramesQueue.front();
    decodedFramesQueue.pop();
    return frame;
}


int FrameDecoder::decodeFrame(AVPacket* pkt)
{
    int ret;
    AVFrame* frame = av_frame_alloc();

    ret = avcodec_send_packet(decoder->video_codec_context, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(decoder->video_codec_context, frame);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return -1;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            return -1;
        }

        //TODO:: This is the problematic code
        if(frame)
        {
            // Create a dummy RGB Frame for conversion
            AVFrame* rgbFrame = av_frame_alloc();
            if (!rgbFrame) {
                throw std::runtime_error("Failed to allocate AVFrame");
            }

            rgbFrame->format = AV_PIX_FMT_RGBA;
            rgbFrame->width = WINDOW_WIDTH;
            rgbFrame->height = WINDOW_HEIGHT;

            int frame_ret = av_frame_get_buffer(rgbFrame, 32); // Align buffer to 32 bytes

            if (frame_ret < 0) {
                throw std::runtime_error("Failed to allocate frame buffer");
            }

            convertYUV420PtoRGB(frame, rgbFrame);
            addFrameToQueue(rgbFrame);
            delete frame;
        }
        else
            return -1;
    }
    return 0;
}

int FrameDecoder::rawDataToFrames(ReconstructedPacket* reconstructedPacket)
{
    AVPacket* pkt = av_packet_alloc();
    if (!pkt) {
        fprintf(stderr, "Could not allocate AVPacket\n");
        return -1;
    }

    size_t data_size = reconstructedPacket->payload.size();
    const uint8_t* data = reconstructedPacket->payload.data();
    size_t parsed_bytes = 0;

    int ret = av_parser_parse2(parser, decoder->video_codec_context, &pkt->data, &pkt->size,
                            data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

    if (ret < 0) {
        fprintf(stderr, "Error while parsing\n");
        av_packet_free(&pkt);
        delete reconstructedPacket;
        return -1;
    }

    if (pkt->size > 0) 
    {
        if (decodeFrame(pkt) < 0) {
            fprintf(stderr, "Error decoding frame\n");
            av_packet_free(&pkt);
            delete reconstructedPacket;
            return -1;
        }
    }
    else
    {
        printf("Failed to parse pkt or reached EOF\n");
        av_packet_free(&pkt);
        delete reconstructedPacket;
    }

    //TODO:: Do we need to flush the decoder
    return 0;
}