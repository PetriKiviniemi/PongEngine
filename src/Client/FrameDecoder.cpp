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

    decoder = new StreamingContext;
    parser = new AVCodecParserContext;

    //TODO:: Put the common codec information to the common.hpp
    decoder->video_codec = avcodec_find_decoder(AV_CODEC_ID_MPEG4);
    decoder->video_codec_context = avcodec_alloc_context3(decoder->video_codec);

    if (!decoder->video_codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    if (avcodec_open2(decoder->video_codec_context, decoder->video_codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    parser = av_parser_init(decoder->video_codec->id);
    if (!parser) {
        fprintf(stderr, "parser not found\n");
        exit(1);
    }

    if (!decoder->video_codec_context) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    printf("Decoder prepared!\n");
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

        printf("Queue is not empty\n");

        if(stop_processing && rawDataQueue.empty())
            break;

        ReconstructedPacket* rpkt = getRawPacketFromQueue();
        if(rpkt)
        {
            printf("rawDataToFrames\n");
            rawDataToFrames(rpkt);
            delete rpkt;
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

void print_frame_info(AVFrame *frame) {
    printf("Frame width: %d\n", frame->width);
    printf("Frame height: %d\n", frame->height);
    printf("Data pointers:\n");
    
    for (int i = 0; i < AV_NUM_DATA_POINTERS; i++) {
        if (frame->data[i] != NULL) {
            printf("  Data[%d] size: %d\n", i, frame->linesize[i] * frame->height);
        }
    }
}

int FrameDecoder::decodeFrame(AVPacket* pkt)
{
    char buf[1024];
    int ret;
    AVFrame* frame = nullptr;
    printf("Decoding pkt to frame...\n");
 
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
            exit(1);
        }

        //TODO:: Do something with the frame
        if(frame)
            printf("Frame decoded succesfully!");
        else
            printf("Failed to decode frame!");
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

    printf("ReconstructedPacket to frame frameN: %d payloadSize: %ld \n",
           reconstructedPacket->frameNumber, reconstructedPacket->payload.size());

    size_t data_size = reconstructedPacket->payload.size();
    const uint8_t* data = reconstructedPacket->payload.data();
    size_t parsed_bytes = 0;

    printf("Parsing data, data_size: %ld\n", data_size);

    while(data_size > 0)
    {
        int ret = av_parser_parse2(parser, decoder->video_codec_context, &pkt->data, &pkt->size,
                                data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret < 0) {
            fprintf(stderr, "Error while parsing\n");
            av_packet_free(&pkt);
            delete reconstructedPacket;
            return -1;
        }
        
        data += ret;
        data_size -= ret;

        printf("Parsed %d bytes, remaining data_size: %ld, packet size: %d\n", ret, data_size - ret, pkt->size);

        if (pkt->size) 
        {
            printPacketContent(pkt);
            printf("Decoding frame\n");

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
        }
    }

    //TODO:: Do we need to flush the decoder

    av_packet_free(&pkt);
    delete reconstructedPacket;
    return 0;
}