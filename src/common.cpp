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

void printPacketContent(AVPacket *pkt) {
    printf("Printing packet...\n");
    printf("Packet data:\n");
    printf("  Stream index: %d\n", pkt->stream_index);
    printf("  Presentation timestamp: %ld\n", pkt->pts);
    printf("  Decompression timestamp: %ld\n", pkt->dts);
    printf("  Duration: %ld\n", pkt->duration);
    printf("  Size: %d\n", pkt->size);
    printf("  Flags: %d\n", pkt->flags);
}

/*
 NOTE:: This is just utility function to print YUV420 pixel data to verify RGBA->YUV420p conversion
 This is copied from stackoverflow
*/
void printYUV420pPixels(uint8_t *yuvData, int width, int height) {
    // Assuming `yuvData` is the pointer to YUV420p pixel data
    // `width` and `height` are the dimensions of the image

    int ySize = width * height;
    int uSize = (width / 2) * (height / 2); // U and V planes are half width and height
    int vSize = uSize;

    // Pointer to Y, U, and V planes
    uint8_t *yPlane = yuvData;
    uint8_t *uPlane = yuvData + ySize;
    uint8_t *vPlane = yuvData + ySize + uSize;

    // Print individual pixel values
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            // Calculate pixel index in Y plane
            int yIndex = row * width + col;

            // Get luminance (Y) value
            uint8_t yValue = yPlane[yIndex];

            // Calculate pixel index in U and V planes (half resolution)
            int uIndex = (row / 2) * (width / 2) + (col / 2);
            int vIndex = (row / 2) * (width / 2) + (col / 2);

            // Get chrominance (U and V) values
            uint8_t uValue = uPlane[uIndex];
            uint8_t vValue = vPlane[vIndex];

            // Print pixel values (YUV)
            std::cout << "Pixel (" << row << ", " << col << "): Y=" << static_cast<int>(yValue)
                      << " U=" << static_cast<int>(uValue) << " V=" << static_cast<int>(vValue)
                      << std::endl;
        }
    }
}