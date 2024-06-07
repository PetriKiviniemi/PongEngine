#include <PongEngineUDPClient.hpp>
#include <cstdio>
#include <map>
#include <FrameDecoder.hpp>

PongEngineUDPClient* PongEngineUDPClient::pinstance_{nullptr};
std::mutex PongEngineUDPClient::mutex_;

PongEngineUDPClient *PongEngineUDPClient::GetInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pinstance_ == nullptr)
    {
        pinstance_ = new PongEngineUDPClient();
    }
    return pinstance_;
}

void PongEngineUDPClient::receiveMessages()
{
    std::map<uint32_t, ReconstructedPacket*> reconstructedPackets;

    while (is_receiving)
    {
        // Buffer to receive data
        const size_t buf_size = MAX_PACKET_SIZE + sizeof(UDPHeader);
        char buf[buf_size];
        double ptime;
        auto ret_code = receiver.receive(buf, buf_size, &ptime);


        if (ret_code < 0)
        {
            std::cerr << "Failed to receive bytes!" << std::endl;
            continue;
        }

        // Copy UDP header from buffer
        UDPHeader udpHeader;
        std::memcpy(&udpHeader, buf, sizeof(UDPHeader));

        // Find or create the reconstructed packet
        ReconstructedPacket*& pkt = reconstructedPackets[udpHeader.frameNumber];
        if (!pkt)
        {
            pkt = new ReconstructedPacket;
            pkt->frameNumber = udpHeader.frameNumber;
        }

        // Ensure we do not exceed the buffer size
        size_t payloadStart = sizeof(UDPHeader);

        // Append payload to the packet's payload vector
        pkt->payload.insert(
            pkt->payload.end(),
            buf + payloadStart,
            buf + payloadStart + udpHeader.fragmentSize
        );

        // If all fragments are received, add to the FrameDecoder queue
        if (udpHeader.fragmentNumber == udpHeader.totalFragments - 1)
        {
            //Lets store the first 20 frames into a file
            if (udpHeader.frameNumber < 20)
            {
                std::string fileName = "./TempFrameData/Client_Frame";
                fileName += std::to_string(udpHeader.frameNumber);
                fileName += ".txt";
                saveBytesToFile(pkt->payload.data(), pkt->payload.size(), fileName.c_str());
            }

            if(pkt->payload.size() != udpHeader.totalSize)
                printf("Size missmatch. Fragment lost?\n");

            FrameDecoder::GetInstance()->addRawDataToQueue(pkt);
            reconstructedPackets.erase(udpHeader.frameNumber);
        }
    }

    // Clean up any remaining packets
    for (auto& entry : reconstructedPackets)
    {
        delete entry.second;
    }

    reconstructedPackets.clear();
}
