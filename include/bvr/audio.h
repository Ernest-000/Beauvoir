#pragma once

#include <bvr/config.h>
#include <bvr/common.h>
#include <bvr/collections/string.h>

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

#if !defined(BVR_MAX_AUDIO_TRACKS)
    #define BVR_MAX_AUDIO_TRACKS 10
#endif


#define BVR_AUDIO_TRACK0 0
#define BVR_AUDIO_TRACK1 1
#define BVR_AUDIO_TRACK2 2
#define BVR_AUDIO_TRACK3 3
#define BVR_AUDIO_TRACK4 4
#define BVR_AUDIO_TRACK5 5
#define BVR_AUDIO_TRACK6 6
#define BVR_AUDIO_TRACK7 7
#define BVR_AUDIO_TRACK8 8
#define BVR_AUDIO_TRACK9 9

enum bvr_audio_device_e {
    BVR_AUDIO_DEFAULT_OUTPUT = 0xFFFFFFFFu,
    BVR_AUDIO_DEFAULT_INPUT = 0xFFFFFFFEu
};

enum bvr_audio_format_e {
    BVR_AUDIO_FORMAT_FLOAT,
    BVR_AUDIO_FORMAT_UINT8,
    BVR_AUDIO_FORMAT_INT8,
    BVR_AUDIO_FORMAT_INT16,
    BVR_AUDIO_FORMAT_INT24,
    BVR_AUDIO_FORMAT_INT32 
};


union bvr_audio_client_u {
    struct {
        void* context;
    } win;

    struct {
        void* loop;
        void* context;
        void* core;
        void* stream;        
    } pipewire;
};

struct bvr_audio_command_s {
    uint32 id;

    short* wave;
    uint32 sample_count;
    
    uint8 channels;
    uint8 track_id;
    
    uint16 sample_depth;
};

typedef struct bvr_audio_track_s {
    float gain;
    float pan;
} bvr_audio_track_t;

typedef struct bvr_audio_mixer_s {
    union bvr_audio_client_u client;

    // physical device id
    enum bvr_audio_device_e device_id;

    // number of output channels
    uint8 channels;

    /* frequency of each samples */
    uint32 sample_rate;

    /* maximum amplitude for a sample */
    uint32 sample_depth;

    enum bvr_audio_format_e format;
    
    /* global volume */
    float gain;

    struct {
        bvr_audio_track_t tracks[BVR_MAX_AUDIO_TRACKS];

        uint32 requested_length;
        short pcm[BVR_AUDIO_FRAME_COUNT];
    } master;

    uint32 command_count;
    struct bvr_audio_command_s commands[BVR_MAX_AUDIO_COMMAND];

    bool avail;
} bvr_audio_mixer_t;

typedef struct bvr_audio_s {
    bvr_string_t name;

    short* wave;
    uint32 wave_length;

    uint8 channels;
    uint32 format;
    uint32 sample_rate;
    uint32 sample_depth;

    float duration;
} bvr_audio_t;

int bvr_create_audiof(bvr_audio_t* audio, FILE* file, const char* name);

/**
 * Create a new audio sound from path.
 */
BVR_H_FUNC int bvr_create_audio(bvr_audio_t* audio, const char* path, const char* name){
    BVR_ASSERT(path);
    
    FILE* file = fopen(path, "rb");
    int status = bvr_create_audiof(audio, file, name);
    fclose(file);
    return status;
}

/**
 * Play an audio sound from the start
 */
void bvr_audio_play(bvr_audio_t* audio, uint8 track);

/**
 * Stop an audio sound from playing
 */
void bvr_audio_stop(bvr_audio_t* audio);

void bvr_destroy_audio(bvr_audio_t* audio);

/**
 * Create a new scene audio stream.
 * @param sample_rate stream's audio frequency
 * @param channel number of audio channels
 */
int bvr_create_audio_mixer(bvr_audio_mixer_t* stream, const int sample_rate, const uint8 channels);

/**
 * Initialize a specific audio track
 * @param mixer
 * @param track track index
 * @param volume volume of the track. Value between 0.0 and 100.0
 * @param pan directing the sound more to the left, right, or center speaker where -100.0f is left and 100.0f is right
 */
void bvr_create_audio_track(bvr_audio_mixer_t* mixer, uint8 track, float volume, float pan);

/**
 * @return the number of played bytes
 */
uint32 bvr_audio_do_wave_command(struct bvr_audio_command_s* command);
void bvr_audio_add_wave_command(const struct bvr_audio_command_s* command);

void bvr_destroy_audio_mixer(bvr_audio_mixer_t* stream);
