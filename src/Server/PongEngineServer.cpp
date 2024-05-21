#include "PongEngineServer.hpp"

PongEngineUDPServer* PongEngineUDPServer::pinstance_{nullptr};
std::mutex PongEngineUDPServer::mutex_;

PongEngineUDPServer *PongEngineUDPServer::GetInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pinstance_ == nullptr)
    {
        pinstance_ = new PongEngineUDPServer();
    }
    return pinstance_;
}

//Just a singleton wrapper for the UDPSend6 files
void PongEngineUDPServer::runServer()
{
	std::cout << "Launching UDP server..." << std::endl;
	udp_sender.init(addr, sizeof(addr)/sizeof(char), UDP_PORT);
	std::cout << "UDP server started!" << std::endl;
}

void PongEngineUDPServer::closeServer()
{
	std::cout << "Closing UDP server..." << std::endl;
	udp_sender.closeSock();
	std::cout << "UDP Server closed!" << std::endl;
}

void PongEngineUDPServer::fragmentAndSendPacket(AVPacket *pkt, int frame_index)
{
	// Since we are only ever sending FFMPEG frames as udp packets
	// Use UDPHeader always

	//TODO:: Revise this code
	int numFragments = pkt->size / MAX_PACKET_SIZE;

	// Create the custom header
	UDPHeader udpHeader;
	udpHeader.frameNumber = frame_index;
	udpHeader.totalFragments = numFragments;

	for (int fragmentIndex = 0; fragmentIndex < numFragments; ++fragmentIndex) {
		int fragmentSize = minimum(MAX_PACKET_SIZE, pkt->size - fragmentIndex * MAX_PACKET_SIZE);
		
		udpHeader.fragmentNumber = fragmentIndex;
		udpHeader.payloadSize = fragmentSize;

		// Copy UDP header and payload data into pkt->data
		std::memcpy(pkt->data, &udpHeader, sizeof(UDPHeader));
		std::memcpy(pkt->data + sizeof(UDPHeader), pkt->data + fragmentIndex * MAX_PACKET_SIZE, fragmentSize);
		size_t totalDataSize = sizeof(UDPHeader) + fragmentSize;
		char* charBuff = new char[totalDataSize];
		std::memcpy(charBuff, reinterpret_cast<char*>(pkt->data), totalDataSize);

		// Send UDP packet
		udp_sender.send(charBuff, sizeof(UDPHeader) + fragmentSize);

		delete[] charBuff;

		//TODO:: Sync to framerate (25-30fps)
	}
}