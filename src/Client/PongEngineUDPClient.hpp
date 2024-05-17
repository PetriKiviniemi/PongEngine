#ifndef PONG_ENGINE_CLIENT_HPP
#define PONG_ENGINE_CLIENT_HPP

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
#include <UDPReceive6.h>

//Just a singleton wrapper for the UDPSend6 files
class PongEngineUDPClient{
    private:
        static PongEngineUDPClient* pinstance_;
        static std::mutex mutex_;

		int UDP_PORT = 9090;
        bool is_receiving = false;
        UDPReceive6 receiver;
		std::thread udp_client_thread;
        void receiveMessages();
    protected:
        PongEngineUDPClient() {
            receiver.init(9090);
            is_receiving = true;
        };
        ~PongEngineUDPClient() {
            closeClient();
        };
    public:
        PongEngineUDPClient(PongEngineUDPClient &other) = delete;
        void operator=(const PongEngineUDPClient &) = delete;
        static PongEngineUDPClient *GetInstance();

		void runClient() {
			udp_client_thread = std::thread(&PongEngineUDPClient::receiveMessages, this);
        };
		void closeClient() {
			if (is_receiving)
			{
				is_receiving = false;
				udp_client_thread.join();
                receiver.closeSock();
			}
        };
};

#endif