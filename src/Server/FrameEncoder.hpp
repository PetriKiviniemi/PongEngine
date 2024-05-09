#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <common.hpp>


struct StreamingContext
{
    AVFormatContext *video_format_ctx;
    const AVCodec *video_codec;
    AVCodecContext *video_codec_context;
    AVStream *video_stream;
    AVDictionary *muxer_opts;
    int video_frame_index = 0;
    int frame_rate = 24;
};


class FrameEncoder{
    private:
		StreamingContext *encoder;
		StreamingContext *decoder;
		std::queue<FrameData *> frame_queue;
		std::mutex queue_mtx;
		std::condition_variable queue_cv;
		std::atomic<bool> stop_processing{false};
		std::thread video_processing_thread;

        static FrameEncoder* pinstance_;
        static std::mutex mutex_;
    protected:
        FrameEncoder();
        ~FrameEncoder();
        int prepare_video_encoder();
        void processFrames();
        void addToQueue(FrameData *frameData);
        void encodeFrame(FrameData *frameData);
    public:
        FrameEncoder(FrameEncoder &other) = delete;
        void operator=(const FrameEncoder &) = delete;
        static FrameEncoder *GetInstance();

        int encodeAndAddToQueue(uint8_t* pixelData, uint32_t pixelDataSize);
        FrameData* getFrameFromQueue();
        void cleanUp();
};