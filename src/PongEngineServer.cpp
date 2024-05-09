#include "PongEngineServer.hpp"



//Just a singleton wrapper for the UDPSend6 files
class PongEngineUDPServer{
    private:
        static PongEngineUDPServer* pinstance_;
        static std::mutex mutex_;

		//UDP Server stuff
		int UDP_PORT = 9090;
		UDPSend6 udp_sender;

    public:
        PongEngineUDPServer(PongEngineUDPServer &other) = delete;
        void operator=(const PongEngineUDPServer &) = delete;
        static PongEngineUDPServer *GetInstance(const std::string& value);




		void runServer()
		{
			std::cout << "Launching UDP server..." << std::endl;
			char addr[] = "localhost";
			udp_sender.init(addr, sizeof(addr)/sizeof(char), UDP_PORT);
			std::cout << "UDP server started!" << std::endl;
		}

		void closeServer()
		{
			udp_sender.closeSock();
		}
};