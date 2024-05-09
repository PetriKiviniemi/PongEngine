#include <common.hpp>

//This function was copied from stackoverflow
void flipPixelsVertically(uint8_t *pixels, int width, int height) {
    int bytesPerPixel = 4; // RGBA
    uint8_t *tempRow = new uint8_t[width * bytesPerPixel];

    for (int row = 0; row < height / 2; ++row) {
        int oppositeRow = height - 1 - row;

        for (int col = 0; col < width; ++col) {
            memcpy(tempRow, &pixels[row * width * bytesPerPixel + col * bytesPerPixel], bytesPerPixel);
            memcpy(&pixels[row * width * bytesPerPixel + col * bytesPerPixel],
                   &pixels[oppositeRow * width * bytesPerPixel + col * bytesPerPixel], bytesPerPixel);
            memcpy(&pixels[oppositeRow * width * bytesPerPixel + col * bytesPerPixel], tempRow, bytesPerPixel);
        }
    }

    delete[] tempRow;
}

void printUDPPacketFragment(AVPacket* pkt, int fragmentSize)
{
    std::cout << "Sent UDP packet with custom header and payload:" << std::endl;
    for (int i = 0; i < sizeof(UDPHeader) + fragmentSize; ++i) {
    	std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)pkt->data[i] << " ";
    }
    std::cout << std::dec << std::endl;
}