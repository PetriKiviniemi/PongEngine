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
    // Calculate the number of fragments required
    int numFragments = std::ceil(static_cast<double>(pkt->size) / MAX_PACKET_SIZE);

    // Create the custom header
    UDPHeader udpHeader;
    udpHeader.frameNumber = frame_index;
    udpHeader.totalFragments = numFragments;
	udpHeader.totalSize = pkt->size;

    // For comparison
   // //Lets store the first 20 frames into a file
   // if (udpHeader.frameNumber < 20)
   // {
   //     std::string fileName = "./TempFrameData/Server_Frame";
   //     fileName += std::to_string(frame_index);
   //     fileName += ".txt";
   //     saveBytesToFile(pkt->data, pkt->size, fileName.c_str());
   // }

    for (int fragIdx = 0; fragIdx < numFragments; ++fragIdx) {
        // Determine the size of the current fragment
        int fragmentSize = std::min(MAX_PACKET_SIZE, pkt->size - fragIdx * MAX_PACKET_SIZE);

        udpHeader.fragmentNumber = fragIdx;
		udpHeader.fragmentSize = fragmentSize;

        // Allocate buffer for the UDP packet
        size_t totalDataSize = sizeof(UDPHeader) + fragmentSize;
        char* charBuff = new char[totalDataSize];

        // Copy UDP header into the buffer
        std::memcpy(charBuff, &udpHeader, sizeof(UDPHeader));

        // Copy the fragment payload into the buffer
        std::memcpy(charBuff + sizeof(UDPHeader), pkt->data + fragIdx * MAX_PACKET_SIZE, fragmentSize);

        // Send UDP packet
        udp_sender.send(charBuff, totalDataSize);

        // Free the buffer memory
        delete[] charBuff;

        // TODO: Sync to framerate (25-30fps)
    }
}
