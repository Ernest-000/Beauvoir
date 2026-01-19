#include <bvr/audio.h>

#include <SDL3/SDL_audio.h>

static void bvri_audio_callback(void* stream, SDL_AudioStream* sdl, int additional_amount, int total_amount){
    const bvr_audio_stream_t* self = (bvr_audio_stream_t*)stream;
}

int bvr_create_audio_ouput(bvr_audio_stream_t* stream, const int sample_rate, const uint8 channels){
    BVR_ASSERT(stream);
    BVR_ASSERT(sample_rate > 0);

    SDL_AudioSpec config;
    config.channels = channels;
    config.format = SDL_AUDIO_F32;
    config.freq = sample_rate;

    stream->device_id = BVR_AUDIO_DEFAULT_OUTPUT;
    stream->context = SDL_OpenAudioDeviceStream(
        BVR_AUDIO_DEFAULT_OUTPUT, &config, 
        bvri_audio_callback, stream
    );

    BVR_ASSERT(stream->context);

    stream->avail = true;
    return BVR_TRUE;
}

void bvr_destroy_audio_stream(bvr_audio_stream_t* stream){
    BVR_ASSERT(stream);

    SDL_CloseAudioDevice(stream->device_id);
    stream->context = NULL;
    stream->avail = false;
}

void bvr_audio_play(bvr_audio_stream_t* stream){
}

void bvr_audio_stop(bvr_audio_stream_t* stream){
}
