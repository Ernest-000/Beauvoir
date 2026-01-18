#include <BVR/config.h>
#include <BVR/common.h>

#ifndef BVR_DEFAULT_SAMPLE_RATE
    #define BVR_DEFAULT_SAMPLE_RATE 44100
#endif

enum bvr_audio_device_e {
    BVR_AUDIO_DEFAULT_OUTPUT = 0xFFFFFFFFu,
    BVR_AUDIO_DEFAULT_INPUT = 0xFFFFFFFEu
};

typedef struct bvr_audio_stream_s {
    void* context;
    uint32 device_id;

    bool avail;
} bvr_audio_stream_t;

int bvr_create_audio_ouput(bvr_audio_stream_t* stream, const int sample_rate, const uint8 channels);
void bvr_audio_play(bvr_audio_stream_t* stream);
void bvr_audio_stop(bvr_audio_stream_t* stream);

void bvr_destroy_audio_stream(bvr_audio_stream_t* stream);