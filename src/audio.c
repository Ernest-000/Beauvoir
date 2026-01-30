#include <bvr/audio.h>
#include <bvr/scene.h>

#include <bvr/window.h>

#include <SDL3/SDL_audio.h>

#include <malloc.h>
#include <memory.h>

#define BVR_AUDIO_FORMAT SDL_AUDIO_S16

void bvr_create_audio(bvr_audio_t* audio, const bvr_audio_stream_t* stream, const char* file){
    BVR_ASSERT(audio);
    BVR_ASSERT(file);

    SDL_AudioSpec config;
    config.channels = stream->channels;
    config.format = BVR_AUDIO_FORMAT;
    config.freq = stream->sample_rate;
    
    BVR_ASSERT(SDL_LoadWAV(file, &config, &audio->wave, &audio->wave_length));
}

void bvr_audio_play(bvr_audio_t* audio){
    BVR_ASSERT(audio);
    BVR_ASSERT(audio->wave);

    struct bvr_audio_command_s cmd;
    cmd.wave = audio->wave;
    cmd.wave_length = audio->wave_length;

    bvr_audio_add_wave_command(&cmd);
}

void bvr_destroy_audio(bvr_audio_t* audio){
    BVR_ASSERT(audio);

    free(audio->wave);
    audio->wave = NULL;
}

static void bvri_audio_callback(void* _stream, SDL_AudioStream* sdl, int additional_amount, int total_amount){
    const bvr_audio_stream_t* stream = (bvr_audio_stream_t*)_stream;

    bvr_audio_push(stream);
}

int bvr_create_audio_stream(bvr_audio_stream_t* stream, const int sample_rate, const uint8 channels){
    BVR_ASSERT(stream);
    BVR_ASSERT(sample_rate > 0);

    SDL_AudioSpec config;
    config.channels = channels;
    config.format = BVR_AUDIO_FORMAT;
    config.freq = sample_rate;

    stream->channels = channels;
    stream->sample_rate = sample_rate;
    stream->device_id = BVR_AUDIO_DEFAULT_OUTPUT;

    stream->context = SDL_OpenAudioDeviceStream(
        BVR_AUDIO_DEFAULT_OUTPUT, &config, 
        bvri_audio_callback, stream
    );
    BVR_ASSERT(stream->context);

    memset(stream->commands, 0, sizeof(stream->commands));
    stream->command_count = 0;
    
    SDL_ResumeAudioStreamDevice(stream->context);

    stream->avail = true;
    return BVR_TRUE;
}

void bvr_audio_wave_command(const struct bvr_audio_command_s* command){
    SDL_PutAudioStreamData(
        bvr_get_instance()->audio.context, 
        command->wave, command->wave_length
    );
}

void bvr_audio_add_wave_command(const struct bvr_audio_command_s* command){
    BVR_ASSERT(command);

    if(!bvr_get_instance()->audio.avail){
        return;
    }

    if(bvr_get_instance()->audio.command_count + 1 < BVR_MAX_AUDIO_COMMAND){
        memcpy(
            &bvr_get_instance()->audio.commands[bvr_get_instance()->audio.command_count++],
            command, sizeof(struct bvr_audio_command_s)
        );
    }
}

void bvr_audio_push(bvr_audio_stream_t* stream){
    const int max_sample_per_sec = BVR_SAMPLE_RATE * sizeof(float) / 2;

    for (size_t i = 0; i < stream->command_count; i++)
    {
        if(SDL_GetAudioStreamQueued(stream->context) >= max_sample_per_sec){
            break;
        }

        bvr_audio_wave_command(&stream->commands[i]);
    }

    stream->command_count = 0;
}

void bvr_destroy_audio_stream(bvr_audio_stream_t* stream){
    BVR_ASSERT(stream);

    SDL_CloseAudioDevice(stream->device_id);
    stream->context = NULL;
    stream->avail = false;
}