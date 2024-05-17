#ifndef FRAME_DECODER_HPP
#define FRAME_DECODER_HPP

#include <queue>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <common.hpp>

// Singleton wrapper for decoding frames
class FrameDecoder {
    private:
        static FrameDecoder* pinstance_;
        static std::mutex mutex_;


		StreamingContext* decoder;
        AVCodecParserContext* parser;

        std::queue<ReconstructedPacket*> rawDataQueue;
		std::mutex queue_mtx;

		std::condition_variable queue_cv;
		std::atomic<bool> stop_processing{false};
		std::thread data_processing_thread;

    protected:
        FrameDecoder();
        ~FrameDecoder();

        int prepareDecoder();
        int decodeFrame(AVPacket* pkt);
        int rawDataToFrames(ReconstructedPacket* reconstructedPacket);
        void cleanUp();
    public:
        FrameDecoder(FrameDecoder &other) = delete;
        void operator=(const FrameDecoder &) = delete;
        static FrameDecoder *GetInstance();

        ReconstructedPacket* getRawPacketFromQueue();
        void addRawDataToQueue(ReconstructedPacket* rpkt);
        void processRawPacketData();
};

#endif