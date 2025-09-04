#include "video_reader.hpp"

#include <filesystem>
#include <iostream>

// This function can be used as a replacement for av_err2str.
static const char* av_make_error(int errnum) {
    static char str[AV_ERROR_MAX_STRING_SIZE];
    memset(str, 0, sizeof(str));
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}

bool video_reader_open(VideoReaderState* state, const char* filename) 
{
    // Unpack members of state
    auto& width = state->width;
    auto& height = state->height;
    auto& time_base = state->time_base;
    auto& av_format_ctx = state->av_format_ctx;
    auto& av_codec_ctx = state->av_codec_ctx;
    auto& video_stream_index = state->video_stream_index;
    auto& av_frame = state->av_frame;
    auto& av_packet = state->av_packet;
    auto& r_frame_rate = state->r_frame_rate;

    // Open the file using libavformat
    av_format_ctx = avformat_alloc_context();
    if (!av_format_ctx) {
        printf("Couldn't created AVFormatContext\n");
        return false;
    }

    if (avformat_open_input(&av_format_ctx, filename, NULL, NULL) != 0) {
        printf("Couldn't open video file\n");
        return false;
    }

    // Find the first valid video stream inside the file
    video_stream_index = -1;
    AVCodecParameters* av_codec_params;
    AVCodec* av_codec;
    for (int i = 0; i < int(av_format_ctx->nb_streams); ++i) {
        av_codec_params = av_format_ctx->streams[i]->codecpar;
        av_codec = const_cast<AVCodec*>(avcodec_find_decoder(av_codec_params->codec_id));
        if (!av_codec) {
            continue;
        }
        if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            width = av_codec_params->width;
            height = av_codec_params->height;
            time_base = av_format_ctx->streams[i]->time_base;
            r_frame_rate = av_format_ctx->streams[i]->r_frame_rate;
            break;
        }
    }
    if (video_stream_index == -1) {
        printf("Couldn't find valid video stream inside file\n");
        return false;
    }

    // Set up a codec context for the decoder
    av_codec_ctx = avcodec_alloc_context3(av_codec);
    if (!av_codec_ctx) {
        printf("Couldn't create AVCodecContext\n");
        return false;
    }
    if (avcodec_parameters_to_context(av_codec_ctx, av_codec_params) < 0) {
        printf("Couldn't initialize AVCodecContext\n");
        return false;
    }
    if (avcodec_open2(av_codec_ctx, av_codec, NULL) < 0) {
        printf("Couldn't open codec\n");
        return false;
    }

    av_frame = av_frame_alloc();
    if (!av_frame) {
        printf("Couldn't allocate AVFrame\n");
        return false;
    }
    av_packet = av_packet_alloc();
    if (!av_packet) {
        printf("Couldn't allocate AVPacket\n");
        return false;
    }

    return true;
}

bool video_reader_read_frame(VideoReaderState* state) 
{
    // Unpack members of state
    auto& width = state->width;
    auto& height = state->height;
    auto& av_format_ctx = state->av_format_ctx;
    auto& av_codec_ctx = state->av_codec_ctx;
    auto& video_stream_index = state->video_stream_index;
    auto& av_frame = state->av_frame;
    auto& av_packet = state->av_packet;

    // Decode one frame
    int response;
    while (av_read_frame(av_format_ctx, av_packet) >= 0) {
        if (av_packet->stream_index != video_stream_index) {
            av_packet_unref(av_packet);
            continue;
        }

        response = avcodec_send_packet(av_codec_ctx, av_packet);
        if (response < 0) {
            printf("Failed to decode packet: %s\n", av_make_error(response));
            return false;
        }

        response = avcodec_receive_frame(av_codec_ctx, av_frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            av_packet_unref(av_packet);
            continue;
        } else if (response < 0) {
            printf("Failed to decode packet: %s\n", av_make_error(response));
            return false;
        }

        av_packet_unref(av_packet);
        break;
    }

    return true;
}

bool video_reader_next_frame(VideoReaderState* state) 
{
    // Unpack members of state
    auto& width = state->width;
    auto& height = state->height;
    auto& av_format_ctx = state->av_format_ctx;
    auto& av_codec_ctx = state->av_codec_ctx;
    auto& video_stream_index = state->video_stream_index;
    auto& av_frame = state->av_frame;
    auto& av_packet = state->av_packet;

    int64_t target_pts = av_frame->pts + (state->r_frame_rate.den * state->time_base.den) / (int64_t(state->r_frame_rate.num) * state->time_base.num);

    while (av_frame->pts < target_pts)
    {
        video_reader_read_frame(state);
    }

    return true;
}

bool video_reader_seek_frame(VideoReaderState* state, int64_t frame) 
{
    // Unpack members of state
    auto& av_format_ctx = state->av_format_ctx;
    auto& av_codec_ctx = state->av_codec_ctx;
    auto& video_stream_index = state->video_stream_index;
    auto& av_packet = state->av_packet;
    auto& av_frame = state->av_frame;

    --frame;
    
    int64_t pts = (int64_t(frame) * state->r_frame_rate.den * state->time_base.den) / (int64_t(state->r_frame_rate.num) * state->time_base.num);
    av_seek_frame(av_format_ctx, video_stream_index, pts, AVSEEK_FLAG_BACKWARD);
    video_reader_read_frame(state);
    
    while (av_frame->pts < pts)
    {
        video_reader_read_frame(state);
    }

    return true;
}

void video_reader_close(VideoReaderState* state) 
{
    avformat_close_input(&state->av_format_ctx);
    avformat_free_context(state->av_format_ctx);
    av_frame_free(&state->av_frame);
    av_packet_free(&state->av_packet);
    avcodec_free_context(&state->av_codec_ctx);
}

float* get_depth_data(VideoReaderState* state)
{
    auto& av_frame = state->av_frame;

    float* data = new float[av_frame->width * av_frame->height];
    uint16_t* luminances = (uint16_t*)av_frame->data[0];
    for (int x = 0; x < av_frame->width; ++x)
    {
        for (int y = 0; y < av_frame->height; ++y)
        {
            float luminance = (float) (double(luminances[y * av_frame->width + x]) / 1024);
            data[y * av_frame->width + x] = luminance;
        }
    }
    return data;
}

unsigned char* get_color_data(VideoReaderState* state)
{
    auto& av_frame = state->av_frame;

    unsigned char* data = new unsigned char[av_frame->width * av_frame->height * 3];
    for (int x = 0; x < av_frame->width; ++x)
    {
        for (int y = 0; y < av_frame->height; ++y)
        {
            int luminance = av_frame->data[0][y * av_frame->linesize[0] + x];
            
            int subsample_x = x >> 1;
            int subsample_y = y >> 1;

            int cb = av_frame->data[1][subsample_y * av_frame->linesize[1] + subsample_x] - 128;
            int cr = av_frame->data[2][subsample_y * av_frame->linesize[2] + subsample_x] - 128;

            // RGB conversion
            float r = luminance + 1.402f * cr;
            float g = luminance - 0.344136f * cb - 0.714136f * cr;
            float b = luminance + 1.772f * cb;

            auto clamp = [](float v) -> unsigned char {
                return static_cast<unsigned char>(std::max(0.f, std::min(255.f, v + 0.5f)));
            };

            data[y * av_frame->width * 3 + x * 3     ] = clamp(r);
            data[y * av_frame->width * 3 + x * 3 + 1 ] = clamp(g);
            data[y * av_frame->width * 3 + x * 3 + 2 ] = clamp(b);
        }
    }
    return data;
}

float* get_depth_frame(std::string filepath, int64_t frame)
{
    VideoReaderState vr_state;
    if (!video_reader_open(&vr_state, filepath.c_str())){
        printf("Couldn't open video file (make sure you set a video file that exists)\n");
        return NULL;
    }

    video_reader_seek_frame(&vr_state, frame);

    float* data = get_depth_data(&vr_state);

    video_reader_close(&vr_state);

    return data;
}

unsigned char* get_colored_frame(std::string filepath, int64_t frame)
{
    VideoReaderState vr_state;
    if (!video_reader_open(&vr_state, filepath.c_str())){
        printf("Couldn't open video file (make sure you set a video file that exists)\n");
        return NULL;
    }

    video_reader_seek_frame(&vr_state, frame);

    unsigned char* data = get_color_data(&vr_state);

    video_reader_close(&vr_state);

    return data;
}

unsigned char* get_luminance(VideoReaderState* state)
{
    return state->av_frame->data[0];
}

unsigned char* get_chrominance(VideoReaderState* state)
{
    auto& av_frame = state->av_frame;

    size_t nrPixels = av_frame->width/2 * av_frame->height/2;
    unsigned char* data = new unsigned char[nrPixels*2];
    for (int pixel = 0; pixel < nrPixels; ++pixel)
    {
        unsigned char cb = av_frame->data[1][pixel];
        unsigned char cr = av_frame->data[2][pixel];
        data[pixel*2    ] = cb;
        data[pixel*2 + 1] = cr;
    }
    return data;
}