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
    printf("Receiving\n");
    std::map<uint32_t, ReconstructedPacket*> reconstructedPackets;
    while (is_receiving)
    {
        // Simply store to buffer first, then copy bytes over
        UDPHeader udpHeader;
        char buf[MAX_PACKET_SIZE];
        double ptime;
        auto ret_code = receiver.receive(buf, sizeof buf, &ptime);

        if (ret_code < 0)
        {
            std::cerr << "Failed to receive bytes!" << std::endl;
            continue;
        }

        std::memcpy(&udpHeader, buf, sizeof(UDPHeader));

        ReconstructedPacket* pkt = reconstructedPackets[udpHeader.frameNumber];

        //If packet does not exist, allocate packet
        if(!pkt)
        {
            pkt = new ReconstructedPacket;
            pkt->frameNumber = udpHeader.frameNumber;
            reconstructedPackets[udpHeader.frameNumber] = pkt;
        }

        size_t payloadStart = sizeof(UDPHeader);

        // Insert the fragment into the reconstructed packet data vector
        pkt->payload.insert(
            pkt->payload.end(),
            reinterpret_cast<uint8_t *>(&udpHeader) + payloadStart,
            reinterpret_cast<uint8_t *>(&udpHeader) + payloadStart + udpHeader.payloadSize
        );

        // If all the fragments are collected, simply then decode the frame
        if (udpHeader.fragmentNumber == udpHeader.totalFragments - 1)
        {
            //FrameDecoder takes care of deleting the pointers
            //Probably not a good practice
            FrameDecoder::GetInstance()->addRawDataToQueue(pkt);
            reconstructedPackets.erase(pkt->frameNumber);
        }
    }
}