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
        std::vector<std::thread> udp_client_threads;
        void receiveFrameMessages();
        void receiveInputMessages();
    protected:
        PongEngineUDPClient() {
            receiver.init(UDP_PORT);
            is_receiving = true;
        };
        ~PongEngineUDPClient() {
            closeClient();
        };
    public:
        PongEngineUDPClient(PongEngineUDPClient &other) = delete;
        void operator=(const PongEngineUDPClient &) = delete;
        static PongEngineUDPClient *GetInstance();

        // Remember to call restartClient if you change the port
        void setClientPort(int port) { UDP_PORT = port;};
        void restartClient() {
            closeClient();
            receiver.init(UDP_PORT);
            is_receiving = true;
        }

		void runVideoStreamClient() {
			udp_client_threads.push_back(std::thread(&PongEngineUDPClient::receiveFrameMessages, this));
        };

        void runInputClient(){
			udp_client_threads.push_back(std::thread(&PongEngineUDPClient::receiveInputMessages, this));
        }

		void closeClient() {
			if (is_receiving)
			{
				is_receiving = false;
                for(int i = 0; i < udp_client_threads.size(); i++)
                {
                    udp_client_threads[i].join();
                }
                receiver.closeSock();
			}
        };
};

#endif