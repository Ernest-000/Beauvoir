#include <BVR/common.h>

#define BVR_DEFAULT_SAMPLE_RATE 44100
#define BVR_DEFAULT_AUDIO_BUFFER_SIZE 64

typedef struct bvr_audio_buffer_s {
    float left_phase;
    float right_phase;
} bvr_audio_buffer_t;

typedef struct bvr_audio_stream_s {
    void* stream;
    int sample_rate;
    int channels;
    int frame_per_buffer;

    bvr_audio_buffer_t buffer;
} bvr_audio_stream_t;

int bvr_create_audio_stream(bvr_audio_stream_t* stream, int sampleRate, int bufsize);
void bvr_audio_play(bvr_audio_stream_t* stream);
void bvr_audio_stop(bvr_audio_stream_t* stream);

void bvr_destroy_audio_stream(bvr_audio_stream_t* stream);