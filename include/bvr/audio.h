#pragma once

#include <bvr/config.h>
#include <bvr/common.h>
#include <bvr/buffer.h>

#ifndef BVR_SAMPLE_RATE
    #define BVR_SAMPLE_RATE 44100
#endif

#if !defined(BVR_MAX_AUDIO_SAMPLES)
    #define BVR_MAX_AUDIO_SAMPLES 512
#endif

#if !defined(BVR_MAX_AUDIO_COMMAND)
    #define BVR_MAX_AUDIO_COMMAND 32
#endif


enum bvr_audio_device_e {
    BVR_AUDIO_DEFAULT_OUTPUT = 0xFFFFFFFFu,
    BVR_AUDIO_DEFAULT_INPUT = 0xFFFFFFFEu
};

struct bvr_audio_command_s {
    uint8* wave;
    uint32 wave_length;
};

typedef struct bvr_audio_stream_s {
    void* context;
    uint32 device_id;

    uint8 channels;
    int sample_rate;

    struct bvr_audio_command_s commands[BVR_MAX_AUDIO_COMMAND];
    uint32 command_count;

    bool avail;
} bvr_audio_stream_t;

typedef struct bvr_audio_s {
    uint8* wave;
    uint32 wave_length;
} bvr_audio_t;

void bvr_create_audio(bvr_audio_t* audio, const bvr_audio_stream_t* stream, const char* file);
void bvr_audio_play(bvr_audio_t* audio);
void bvr_destroy_audio(bvr_audio_t* audio);

int bvr_create_audio_stream(bvr_audio_stream_t* stream, const int sample_rate, const uint8 channels);

void bvr_audio_wave_command(const struct bvr_audio_command_s* command);
void bvr_audio_add_wave_command(const struct bvr_audio_command_s* command);

void bvr_audio_push(bvr_audio_stream_t* self);

void bvr_destroy_audio_stream(bvr_audio_stream_t* stream);
