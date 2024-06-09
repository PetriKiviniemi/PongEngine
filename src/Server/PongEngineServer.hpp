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
#include <raylib.h>
#include <common.hpp>


//Just a singleton wrapper for the UDPSend6 files
class PongEngineUDPServer{
    private:
        static PongEngineUDPServer* pinstance_;
        static std::mutex mutex_;

		int UDP_PORT = 9090;
        char* addr = "localhost";
		UDPSend6 udp_sender;
    protected:
        PongEngineUDPServer() { runServer();};
        ~PongEngineUDPServer() { closeServer();};
    public:
        PongEngineUDPServer(PongEngineUDPServer &other) = delete;
        void operator=(const PongEngineUDPServer &) = delete;
        static PongEngineUDPServer *GetInstance();

        void setAddrAndPort(char* address, int port) { UDP_PORT = port; addr = address;};

		void runServer();
		void closeServer();
        void restartServer() {closeServer(); runServer();};

        //This is the function for sending the FFMPEG packets
		void fragmentAndSendPacket(AVPacket *pkt, int frame_index);

        //This is for sending user input
        void sendUserInput(KeyboardKey key);
};

#endif