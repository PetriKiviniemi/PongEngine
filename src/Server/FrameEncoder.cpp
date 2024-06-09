#include <FrameEncoder.hpp>
#include "PongEngineServer.hpp"

FrameEncoder* FrameEncoder::pinstance_{nullptr};
std::mutex FrameEncoder::mutex_;

const char* video_file_name = "video.mp4";

FrameEncoder *FrameEncoder::GetInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pinstance_ == nullptr)
    {
        pinstance_ = new FrameEncoder();
    }
    return pinstance_;
}

FrameEncoder::FrameEncoder(){
    //Prepare frame encoder
    int ret = prepare_video_encoder();
    if (ret < 0)
    {
        std::cerr << "Failed to prepare video encoder! " << std::endl;
        exit(-1);
    }

    std::cout << "Starting video_processing_thread" << std::endl;
    video_processing_thread = std::thread(&FrameEncoder::processFrames, this);
}

FrameEncoder::~FrameEncoder(){
    cleanUp();
}

void FrameEncoder::cleanUp()
{
    stop_processing = true;
    queue_cv.notify_one(); // Notify processing thread to stop

    if (video_processing_thread.joinable())
    {
        video_processing_thread.join();
    }

    printf("Writing trailer and freeing encoder...\n");
    avcodec_send_frame(encoder->video_codec_context, nullptr);
    av_write_trailer(encoder->video_format_ctx);

    av_dict_free(&encoder->muxer_opts);
    avio_closep(&encoder->video_format_ctx->pb);
    avcodec_free_context(&encoder->video_codec_context);
    avformat_free_context(encoder->video_format_ctx);
    printf("Done\n");
}

FrameData* FrameEncoder::getFrameFromQueue()
{
    if(frame_queue.empty())
        return nullptr;

    FrameData *frameData = frame_queue.front();
    frame_queue.pop();
    return frameData;
}

std::vector<FrameData*> FrameEncoder::getFrameBatchFromQueue(int amount)
{
    if(frame_queue.empty())
        return std::vector<FrameData*>{};
    
    std::vector<FrameData*> data{};
    for(int i = 0; i < amount; i++)
    {
        if(!frame_queue.empty())
        {
            data.push_back(frame_queue.front());
            frame_queue.pop();
        }
    }
    return data;
}

void FrameEncoder::addToQueue(FrameData *frameData)
{
    {
        std::lock_guard<std::mutex> lock(queue_mtx);
        frame_queue.push(frameData);
        encoder->video_frame_index++;
    }

    queue_cv.notify_one();
}

int FrameEncoder::prepare_video_encoder()
{
    std::cout << "Preparing encoder..." << std::endl;
    av_log_set_level(AV_LOG_DEBUG);

    encoder = new StreamingContext;
    encoder->video_frame_index = 0;

    // Add video stream to format context
    avformat_alloc_output_context2(&encoder->video_format_ctx, nullptr, nullptr, video_file_name);
    encoder->video_stream = avformat_new_stream(encoder->video_format_ctx, NULL);
    encoder->video_codec = (AVCodec *)avcodec_find_encoder(AV_CODEC_ID_MPEG4);
    encoder->video_codec_context = avcodec_alloc_context3(encoder->video_codec);
    if (!encoder->video_format_ctx)
    {
        std::cerr << "Error: Failed to allocate format context" << std::endl;
        return -1;
    }
    if (!encoder->video_stream)
    {
        std::cerr << "Error: Failed to create new stream" << std::endl;
        return -1;
    }
    if (!encoder->video_codec)
    {
        std::cerr << "Error: Failed to find video codec" << std::endl;
        return -1;
    }
    if (!encoder->video_codec_context)
    {
        std::cerr << "Error: Failed to allocate codec context" << std::endl;
        return -1;
    }
    if (avio_open(&encoder->video_format_ctx->pb, video_file_name, AVIO_FLAG_WRITE) < 0)
    {
        std::cerr << "Error: Failed to open file for writing!" << std::endl;
        return -1;
    }

    encoder->video_codec_context->codec_id = AV_CODEC_ID_MPEG4;
    encoder->video_codec_context->codec_type = AVMEDIA_TYPE_VIDEO;
    encoder->video_codec_context->pix_fmt = AV_PIX_FMT_YUV420P;
    encoder->video_codec_context->width = WINDOW_WIDTH;
    encoder->video_codec_context->height = WINDOW_HEIGHT;
    encoder->video_codec_context->bit_rate = 1000000; // Bitrate
    encoder->video_codec_context->time_base = {1, 60};
    encoder->video_codec_context->gop_size = 10;
    //Lets not use B Frames for the stream

    // Set codec profile and level
    //encoder->video_codec_context->profile = FF_PROFILE_MPEG4_MAIN;
    //encoder->video_codec_context->level = 4;

    if (encoder->video_format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        encoder->video_format_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    // For streaming in web, optimizing file structure with metadata on top
    // av_dict_set(&encoder->muxer_opts, "movflags", "faststart", 0);

    // Try to open codec after changes
    // copy codec_context params to videostream
    // and write headers to format_context
    if (avcodec_open2(encoder->video_codec_context, encoder->video_codec, NULL) < 0)
    {
        std::cerr << "Error: Could not open codec!" << std::endl;
        return -1;
    }
    if (avcodec_parameters_from_context(encoder->video_stream->codecpar, encoder->video_codec_context) < 0)
    {
        std::cerr << "Error: Could not copy params from context to stream!" << std::endl;
        return -1;
    };
    if (avformat_write_header(encoder->video_format_ctx, NULL) < 0)
    {
        std::cerr << "Error: Failed to write output file headers!" << std::endl;
        return -1;
    }

    std::cout << "Encoder prepared!" << std::endl;
    return 0;
}

void FrameEncoder::processFrames()
{
    while(!stop_processing)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mtx);

            // Wait for the condition to be true using std::condition_variable::wait()
            queue_cv.wait(lock, [&]() {
                return !frame_queue.empty() || stop_processing; 
            });
        }

        if (stop_processing && frame_queue.empty())
            break;
        
        // Process in batch instead of single to reduce queue locking overhead
        // The overhead is probably never an issue but just to make sure
        int frame_count = 10;
        std::vector<FrameData*> frameData = getFrameBatchFromQueue(frame_count);
        for(int i = 0; i < frameData.size(); i++)
        {
            encodeFrame(frameData[i]);
            AVFrame* frame = frameData[i]->frame;
            av_frame_free(&frame);
            delete frameData[i];
        }
    }
}


void FrameEncoder::encodeFrame(FrameData* frameData)
{
    // Validation
    if (!frameData->frame)
    {
        std::cerr << "Error: Frame was null! " << std::endl;
        return;
    }
    if (frameData->frame->format != encoder->video_codec_context->pix_fmt)
    {
        std::cerr << "Error: Frame format mismatch!" << std::endl;
        return;
    }
    if (!encoder->video_codec_context)
        return;

    AVPacket *pkt = av_packet_alloc();
    if (!pkt)
    {
        std::cerr << "Error: Failed to allocate AVPacket" << std::endl;
        system("pause");
    }

    int ret = avcodec_send_frame(encoder->video_codec_context, frameData->frame);
    if (ret < 0)
    {
        std::cerr << "Error receiving packet from codec: " << ret << std::endl;
        av_packet_free(&pkt);
        return;
    }

    int count = 0;

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(encoder->video_codec_context, pkt);

        // Error checks
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            break;
        }
        else if (ret < 0)
        {
            std::cerr << "Error receiving packet from codec: " << ret << std::endl;
            av_packet_free(&pkt);
            return;
        }
        if (!encoder->video_stream)
        {
            std::cerr << "Error: video stream is null!" << std::endl;
            av_packet_free(&pkt);
            return;
        }
        pkt->stream_index = encoder->video_stream->index;
        av_packet_rescale_ts(pkt, encoder->video_codec_context->time_base, encoder->video_stream->time_base);

        // TODO:: We could maybe add the packets into a queue
        // And we could poll the queue with our udp server
        // But for now, sending the packets straight away seems more efficient
        PongEngineUDPServer::GetInstance()->fragmentAndSendPacket(pkt, frameData->frame_index);

        //For writing to a local videofile
        //av_write_frame(encoder->video_format_ctx, pkt);

        av_packet_unref(pkt);
    }

    av_packet_free(&pkt);
}


int FrameEncoder::encodeAndAddToQueue(uint8_t* pixelData, uint32_t pixelDataSize)
{
    //Videoencoder is already prepared in the constructor
    //Now convert the pixel data from RGBA to YUV
    AVFrame *frame = av_frame_alloc();

    if (!frame)
    {
        std::cout << "Could not allocate memory for frame!" << std::endl;
        return -1;
    }

    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = WINDOW_WIDTH;
    frame->height = WINDOW_HEIGHT;
    if (av_frame_get_buffer(frame, 32) < 0)
    {
        std::cerr << "Failed to allocate frame buffer! " << std::endl;
        return -1;
    };

    SwsContext *sws_ctx = sws_getContext(
        WINDOW_WIDTH, WINDOW_HEIGHT, AV_PIX_FMT_RGBA,
        WINDOW_WIDTH, WINDOW_HEIGHT, AV_PIX_FMT_YUV420P,
        SWS_BICUBIC, nullptr, nullptr, nullptr);

    uint8_t *src_data[1] = {pixelData};
    int src_linesize[1] = {WINDOW_WIDTH * 4};
    int scale_ret = sws_scale(sws_ctx, src_data, src_linesize, 0, WINDOW_HEIGHT,
                                frame->data, frame->linesize);

    if (scale_ret <= 0)
    {
        std::cerr << "Failed to scale the image to frame" << std::endl;
        return -1;
    }

    sws_freeContext(sws_ctx);

    FrameData *frameData = new FrameData;
    frameData->frame = frame;
    frameData->frame_index = encoder->video_frame_index;
    addToQueue(frameData);
    return 0;
}
