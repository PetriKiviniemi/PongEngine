#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <cstdint>

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
#include <libavutil/avutil.h>
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
	uint32_t totalSize;
    uint32_t fragmentSize;
};

struct FrameData
{
    AVFrame *frame;
    int frame_index;
};

struct StreamingContext
{
    AVFormatContext *video_format_ctx;
    const AVCodec *video_codec;
    AVCodecContext *video_codec_context;
    AVStream *video_stream;
    AVDictionary *muxer_opts;
    int video_frame_index = 0;
    int frame_rate = 25;
};

// Raw packet data reconstructed into struct with metadata
struct ReconstructedPacket
{
	uint32_t frameNumber;
	std::vector<uint8_t> payload;
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

void convertYUV420PtoRGB(AVFrame* srcFrame, AVFrame* dstFrame);

void printUDPPacketFragment(AVPacket* pkt, int fragmentSize);

void printHexDump(const uint8_t* buffer, size_t size);

void printYUV420pPixels(uint8_t *yuvData, int width, int height);

void printRGBAPixels(uint8_t* rgbaData, int width, int height);

void printPacketContent(AVPacket *pkt);

void saveBytesToFile(const uint8_t* bytes, size_t size, const std::string& filePath);

#endif
