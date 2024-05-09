#ifndef COMMON_HPP
#define COMMON_HPP
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

struct FrameData
{
    AVFrame *frame;
    int frame_index;
};

#endif