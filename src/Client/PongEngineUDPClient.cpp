#include <PongEngineUDPClient.hpp>
#include <cstdio>
#include <map>

struct ReconstructedPacket
{
	uint32_t frameNumber;
	std::vector<uint8_t> payload;
};

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
    std::cout << "Receiving ..." << std::endl;
    std::map<uint32_t, ReconstructedPacket> reconstructedPackets;
    while (is_receiving)
    {
        // Since I am too lazy to change the UDP receiver function, simply store to buffer first, then copy bytes over
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

        auto &pkt = reconstructedPackets[udpHeader.frameNumber];
        pkt.frameNumber = udpHeader.frameNumber;

        size_t payloadStart = sizeof(UDPHeader);

        // Insert the fragment into the reconstructed packet data vector
        pkt.payload.insert(pkt.payload.end(),
                            reinterpret_cast<uint8_t *>(&udpHeader) + payloadStart,
                            reinterpret_cast<uint8_t *>(&udpHeader) + payloadStart + udpHeader.payloadSize);

        // If all the fragments are collected, simply then decode the frame
        if (pkt.payload.size() >= udpHeader.totalFragments * MAX_PACKET_SIZE)
        {
            std::cout << pkt.payload.data() << std::endl;
            reconstructedPackets.erase(udpHeader.frameNumber);
        }
    }
}