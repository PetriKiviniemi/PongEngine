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

class FrameEncoder{
    private:
		StreamingContext *encoder;
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
        std::vector<FrameData*> getFrameBatchFromQueue(int amount);
        void cleanUp();
};