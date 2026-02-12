#pragma once

#include <bvr/config.h>
#include <bvr/common.h>
#include <bvr/buffer.h>

#define BVR_AUDIO_MONO 1
#define BVR_AUDIO_STEREO 2

#ifndef BVR_SAMPLE_RATE
    #define BVR_SAMPLE_RATE 44100
#endif

#if !defined(BVR_AUDIO_FRAME_COUNT)
    // sample count is doubled because we usually use 
    // two channels (Left + Right)
    #define BVR_AUDIO_FRAME_COUNT (4096 * 2U)
#endif

#if !defined(BVR_MAX_AUDIO_COMMAND)
    #define BVR_MAX_AUDIO_COMMAND 32
#endif

#define BVR_AUDIO_FLOAT 0x8120u
#define BVR_AUDIO_UINT8 0x0008u
#define BVR_AUDIO_INT8  0x8008u
#define BVR_AUDIO_INT16 0x8010u
#define BVR_AUDIO_INT32 0x8020u

enum bvr_audio_device_e {
    BVR_AUDIO_DEFAULT_OUTPUT = 0xFFFFFFFFu,
    BVR_AUDIO_DEFAULT_INPUT = 0xFFFFFFFEu
};

struct bvr_audio_command_s {
    short* wave;
    uint32 sample_count;
    
    uint8 channels;
    uint32 sample_depth;
};

typedef struct bvr_audio_mixer_s {
    void* context;
    uint32 device_id;

    uint8 channels;

    /* frequency of each samples */
    uint32 sample_rate;

    /* maximum amplitude for a sample */
    uint32 sample_depth;
    
    float gain;

    struct {

        uint32 avail_buffer_length;
        short pcm[BVR_AUDIO_FRAME_COUNT];
    } master;

    uint32 command_count;
    struct bvr_audio_command_s commands[BVR_MAX_AUDIO_COMMAND];

    bool avail;
} bvr_audio_mixer_t;

typedef struct bvr_audio_s {
    short* wave;
    uint32 wave_length;

    uint8 channels;
    uint32 format;
    uint32 sample_rate;
    uint32 sample_depth;

    float duration;
} bvr_audio_t;

int bvr_create_audiof(bvr_audio_t* audio, FILE* file);

/**
 * Create a new audio sound from path.
 */
BVR_H_FUNC int bvr_create_audio(bvr_audio_t* audio, const char* path){
    BVR_ASSERT(path);
    
    FILE* file = fopen(path, "rb");
    int status = bvr_create_audiof(audio, file);
    fclose(file);
    return status;
}

/**
 * Play an audio sound
 */
void bvr_audio_play(bvr_audio_t* audio);

void bvr_destroy_audio(bvr_audio_t* audio);

/**
 * Create a new scene audio stream.
 * @param sample_rate stream's audio frequency
 * @param channel number of audio channels
 */
int bvr_create_audio_mixer(bvr_audio_mixer_t* stream, const int sample_rate, const uint8 channels);

/**
 * @return the number of played bytes
 */
uint32 bvr_audio_do_wave_command(struct bvr_audio_command_s* command);
void bvr_audio_add_wave_command(const struct bvr_audio_command_s* command);

void bvr_destroy_audio_mixer(bvr_audio_mixer_t* stream);
