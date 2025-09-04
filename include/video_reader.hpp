#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <inttypes.h>
}
#include <string>

struct VideoReaderState {
    int width, height;
    AVRational time_base;
    AVRational r_frame_rate;
    AVFormatContext* av_format_ctx;
    AVCodecContext* av_codec_ctx;
    int video_stream_index;
    AVFrame* av_frame;
    AVPacket* av_packet;
};

bool video_reader_open(VideoReaderState* state, const char* filename);
bool video_reader_read_frame(VideoReaderState* state);
bool video_reader_next_frame(VideoReaderState* state);
bool video_reader_seek_frame(VideoReaderState* state, int64_t frame);
void video_reader_close(VideoReaderState* state);

float* get_depth_data(VideoReaderState* state);
unsigned char* get_color_data(VideoReaderState* state);

float* get_depth_frame(std::string filepath, int64_t frame);
unsigned char* get_colored_frame(std::string filepath, int64_t frame);

unsigned char* get_luminance(VideoReaderState* state);
unsigned char* get_chrominance(VideoReaderState* state);
