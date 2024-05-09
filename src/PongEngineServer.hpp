#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include "UDPSend6.h"
#include <chrono>
#include <algorithm>
#include <cstring>
#include <filesystem>

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


//Custom min function for linux
template<typename T>
T minimum(T a, T b) {
    return (a < b) ? a : b;
}
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