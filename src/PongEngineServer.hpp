#ifndef PONG_ENGINE_SERVER_HPP
#define PONG_ENGINE_SERVER_HPP

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
#include <UDPSend6.h>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <common.hpp>

//Custom min function for linux
#define MAX_PACKET_SIZE 1400

//Just a singleton wrapper for the UDPSend6 files
class PongEngineUDPServer{
    private:
        static PongEngineUDPServer* pinstance_;
        static std::mutex mutex_;

		int UDP_PORT = 9090;
        char* addr = "localhost";
		UDPSend6 udp_sender;
    protected:
        PongEngineUDPServer() {runServer();};
        ~PongEngineUDPServer() {closeServer();};
    public:
        PongEngineUDPServer(PongEngineUDPServer &other) = delete;
        void operator=(const PongEngineUDPServer &) = delete;
        static PongEngineUDPServer *GetInstance();

		void runServer();
		void closeServer();
		void fragmentAndSendPacket(AVPacket *pkt, int frame_index);
};

#endif