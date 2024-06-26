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

void printRGBAPixels(uint8_t* rgbaData, int width, int height) {
    // Assuming `rgbaData` is the pointer to RGBA pixel data
    // `width` and `height` are the dimensions of the image

    // RGBA format has 4 bytes per pixel: R, G, B, and A
    int numChannels = 4;

    // Print individual pixel values
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            // Calculate pixel index
            int pixelIndex = (row * width + col) * numChannels;

            // Get RGBA values
            uint8_t rValue = rgbaData[pixelIndex];
            uint8_t gValue = rgbaData[pixelIndex + 1];
            uint8_t bValue = rgbaData[pixelIndex + 2];
            uint8_t aValue = rgbaData[pixelIndex + 3];

            // Print pixel values (RGBA)
            std::cout << "Pixel (" << row << ", " << col << "): "
                      << "R=" << static_cast<int>(rValue) << " "
                      << "G=" << static_cast<int>(gValue) << " "
                      << "B=" << static_cast<int>(bValue) << " "
                      << "A=" << static_cast<int>(aValue)
                      << std::endl;
        }
    }
}

void saveBytesToFile(const uint8_t* bytes, size_t size, const std::string& filePath)
{
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile)
    {
        std::cerr << "Failed to open file for writing: " << filePath << std::endl;
        return;
    }

    outFile.write(reinterpret_cast<const char*>(bytes), size);
    if (!outFile)
    {
        std::cerr << "Failed to write bytes to file: " << filePath << std::endl;
    }
    
    outFile.close();
    if (!outFile)
    {
        std::cerr << "Failed to close file: " << filePath << std::endl;
    }
}

void printHexDump(const uint8_t* buffer, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void convertFramePxFMT(AVFrame* srcFrame, AVFrame* dstFrame, AVPixelFormat inputFmt, AVPixelFormat outputFmt) {
    SwsContext* swsContext = sws_getContext(
        srcFrame->width, srcFrame->height, inputFmt,
        dstFrame->width, dstFrame->height, outputFmt,
        0, nullptr, nullptr, nullptr);

    if (!swsContext) {
        throw std::runtime_error("Failed to initialize SwsContext");
    }

    sws_scale(swsContext,
        srcFrame->data, srcFrame->linesize, 0,
        srcFrame->height, dstFrame->data, dstFrame->linesize
    );

    sws_freeContext(swsContext);
}