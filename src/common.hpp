#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <iomanip>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/log.h>
#include <libavutil/timestamp.h>
#include <libavutil/channel_layout.h>
#include <libavutil/imgutils.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
}

#define WINDOW_HEIGHT 800
#define WINDOW_WIDTH 800
#define MAX_PACKET_SIZE 1400

struct UDPHeader {
    uint32_t frameNumber;    
    uint32_t fragmentNumber; 
    uint32_t totalFragments; 
	uint32_t payloadSize;
};

struct FrameData
{
    AVFrame *frame;
    int frame_index;
};

template<typename T>
T minimum(T a, T b) {
    return (a < b) ? a : b;
}

// Since raylib textures use different coordinate systems, we have to flip
// the pixel data before passing to FFMPEG to encode
// TODO:: If there is no such functionality provided, we could utilize shaders to
// Flip the texture, and then the calculations would be done on GPU instead of CPU
void flipPixelsVertically(uint8_t *pixels, int width, int height); 

void printUDPPacketFragment(AVPacket* pkt, int fragmentSize);

#endif
