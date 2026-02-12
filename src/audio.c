#include <bvr/audio.h>
#include <bvr/scene.h>

#include <bvr/file.h>
#include <bvr/window.h>

#include <SDL3/SDL_audio.h>

#include <memory.h>

#define BVR_AUDIO_FORMAT BVR_AUDIO_INT16
#define BVR_AUDIO_NO_SIGNAL (0)

static void bvri_audio_callback(void* _stream, SDL_AudioStream* sdl, int additional_amount, int total_amount);

#ifndef BVR_NO_WAV

static int bvri_is_wav(FILE* file){
    uint8 sig[4];
    fseek(file, 0, SEEK_SET);
    fread(sig, sizeof(uint8), 4, file);

    return strncmp(sig, "RIFF", 4) == 0;
}

struct bvri_wavheader_s {
    uint8 sig[4];
    uint32 size;

    // WAVE
    uint8 cdda[4]; 

};

struct bvri_wavchunk_s {
    uint8 sig[4];
    uint32 size;
};

struct bvri_wavfmt_s {
    // bloc signature
    struct bvri_wavchunk_s chunk;

    uint16 audio_format;
    uint16 channel_count;
    uint32 sample_rate;
    uint32 byte_per_sec;
    uint16 bloc_align;
    uint16 bits_per_sample;
};

/*
https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
*/
static int bvri_load_wav(bvr_audio_t* audio, FILE* file){
    BVR_ASSERT(audio);
    BVR_ASSERT(file);

    uint8* wave_bytes = NULL;
    struct bvri_wavheader_s header;
    struct bvri_wavfmt_s fmt;
    struct bvri_wavchunk_s chunk;
    
    uint32 unpacked_bytes;

    fseek(file, 0, SEEK_SET);
    fread(&header, sizeof(struct bvri_wavheader_s), 1, file);
    fread(&fmt.chunk, sizeof(struct bvri_wavchunk_s), 1, file);

    if(!BVR_STRNCMP(fmt.chunk.sig, "fmt", 3)){
        BVR_PRINT("enable to find fmt chunk!");
        return BVR_FALSE;
    }

    // read fmt informations
    fmt.audio_format = bvr_fread16_le(file);
    fmt.channel_count = bvr_fread16_le(file);
    fmt.sample_rate = bvr_fread32_le(file);
    fmt.byte_per_sec = bvr_fread32_le(file);
    fmt.bloc_align = bvr_fread16_le(file);
    fmt.bits_per_sample = bvr_fread16_le(file);

    // search data chunk
    while (!feof(file))
    {
        fread(&chunk, sizeof(struct bvri_wavchunk_s), 1, file);
        
        if(BVR_STRNCMP(chunk.sig, "data", 4)){
            break;
        }

        fseek(file, chunk.size, SEEK_CUR);
    }

    audio->wave = malloc(chunk.size);
    audio->wave_length = chunk.size;
    audio->channels = fmt.channel_count;
    audio->sample_rate = fmt.sample_rate;
    audio->sample_depth = fmt.bits_per_sample / 8;
    audio->duration = chunk.size / audio->sample_depth;

    switch (fmt.bits_per_sample)
    {
    case 8:  audio->format = BVR_AUDIO_INT8; break;
    case 16: audio->format = BVR_AUDIO_INT16; break;
    case 32: audio->format = BVR_AUDIO_INT32; break;
    
    default:
        audio->format = BVR_AUDIO_INT8;
        break;
    }

    // copy raw wav to data
    unpacked_bytes = fread(audio->wave, sizeof(uint8), chunk.size, file);
    BVR_ASSERT(chunk.size == unpacked_bytes);

    return BVR_TRUE;
}

#endif

static void bvri_quantize_audio(bvr_audio_t* audio, const bvr_audio_mixer_t* mixer){
    BVR_ASSERT(audio);

    if(!audio->wave){
        return;
    }

    if(audio->sample_rate != mixer->sample_rate){
        BVR_PRINT("sample rates doesn't match");
    }

    if(audio->sample_depth != mixer->sample_depth){
        BVR_PRINT("sample depth doesn't match");
    }

    // BVR_PRINTF(
    //     "audio infos: \nduration: %fs\nsample rate: %i\nsample depth: %i\nchannels: %i\n", 
    //     audio->duration, audio->sample_rate, audio->sample_depth, audio->channels
    // );
}

int bvr_create_audiof(bvr_audio_t* audio, FILE* file){
    BVR_ASSERT(audio);
    BVR_ASSERT(file);

    // check for audio stream
    bvr_audio_mixer_t* mixer = &bvr_get_instance()->audio;
    BVR_ASSERT(mixer->avail);
    
    int status = 0;

    audio->wave = NULL;
    audio->wave_length = 0;
    audio->channels = 0;
    audio->sample_rate = 0;
    audio->sample_depth = 0;

#ifndef BVR_NO_WAV
    if(bvri_is_wav(file)){
        status |= bvri_load_wav(audio, file);
    }
#endif

    bvri_quantize_audio(audio, mixer);

    return status;
}

void bvr_audio_play(bvr_audio_t* audio){
    BVR_ASSERT(audio);
    BVR_ASSERT(audio->wave);

    struct bvr_audio_command_s cmd;
    cmd.wave = audio->wave;
    cmd.sample_depth = audio->sample_depth;
    cmd.sample_count = audio->wave_length / cmd.sample_depth;
    cmd.channels = audio->channels;

    bvr_audio_add_wave_command(&cmd);
}

void bvr_destroy_audio(bvr_audio_t* audio){
    BVR_ASSERT(audio);

    free(audio->wave);
    audio->wave = NULL;
}

static void bvri_audio_callback(void* _stream, SDL_AudioStream* sdl, int additional_amount, int total_amount){
    uint32 done = 0;
    bvr_audio_mixer_t* mixer = (bvr_audio_mixer_t*)_stream;
    mixer->master.avail_buffer_length = MIN(additional_amount, BVR_AUDIO_FRAME_COUNT * sizeof(short));

    // clear previous audio buffer
    memset(mixer->master.pcm, 0, BVR_AUDIO_FRAME_COUNT * sizeof(short));
    
    for (size_t i = 0; i < mixer->command_count; i++)
    {
        bvr_audio_do_wave_command(&mixer->commands[i]);

        if (mixer->commands[i].sample_count > 0) {
            // if we this sample is not finished
            mixer->commands[done++] = mixer->commands[i];
        }
    }

    mixer->command_count = done;

    SDL_PutAudioStreamData(mixer->context, mixer->master.pcm, mixer->master.avail_buffer_length);
}

int bvr_create_audio_mixer(bvr_audio_mixer_t* mixer, const int sample_rate, const uint8 channels){
    BVR_ASSERT(mixer);
    BVR_ASSERT(sample_rate > 0);

    SDL_AudioSpec config;
    config.channels = channels;
    config.format = BVR_AUDIO_FORMAT;
    config.freq = sample_rate;

    mixer->channels = channels;
    mixer->sample_rate = sample_rate;
    mixer->sample_depth = 16;
    mixer->device_id = BVR_AUDIO_DEFAULT_OUTPUT;
    mixer->gain = 0.5f;
    mixer->command_count = 0;

    mixer->context = SDL_OpenAudioDeviceStream(
        BVR_AUDIO_DEFAULT_OUTPUT, &config, 
        bvri_audio_callback, mixer
    );
    BVR_ASSERT(mixer->context);
    
    SDL_ResumeAudioStreamDevice(mixer->context);

    mixer->avail = true;
    return BVR_TRUE;
}

/**
 * Because of how audio works, we might need to follow the same way as real sound mixers
 * https://sound-au.com/articles/audio-mixing.htm
 * 
 * https://lisyarus.github.io/blog/posts/audio-mixing.html
 */
uint32 bvr_audio_do_wave_command(struct bvr_audio_command_s* command){
    bvr_audio_mixer_t* mixer = &bvr_get_instance()->audio;
    uint32 mixing_channel = mixer->channels;

    // get the available mixing sample length
    uint32 wave_samples = MIN(command->sample_count, mixer->master.avail_buffer_length / sizeof(short));

    // get available sample count
    uint32 frame_count = wave_samples / command->channels;

    for (uint32 s = 0; s < frame_count; s++)
    {
        // add audio's amplitude to previous master values 
        short left = (mixer->master.pcm[mixing_channel * s] + command->wave[command->channels * s]);
        short right = (mixer->master.pcm[mixing_channel * s + 1] + command->wave[command->channels * s + 1]);
        
        // clip audio values
        left = clampi(left, BVR_INT16_MIN, BVR_INT16_MAX) * mixer->gain;
        right = clampi(right, BVR_INT16_MIN, BVR_INT16_MAX) * mixer->gain;

        // when master if mono
        mixer->master.pcm[mixing_channel * s + 0] = left;

        // when master is stereo
        if(mixing_channel > 1){
            mixer->master.pcm[mixing_channel * s + 1] = left;
    
            // when audio clip is stereo
            if(command->channels > 1){
                mixer->master.pcm[mixing_channel * s + 1] = right;
            }
        }
    }

    command->wave += wave_samples;
    command->sample_count -= wave_samples;

    return wave_samples;
}

void bvr_audio_add_wave_command(const struct bvr_audio_command_s* command){
    BVR_ASSERT(command);

    // when there is no initialized instance
    if(!bvr_get_instance()->audio.avail){
        return;
    }

    // when there is no space for a new command
    if(bvr_get_instance()->audio.command_count + 1 >= BVR_MAX_AUDIO_COMMAND){
        return;
    }

    // try to find an available command storing space
    memcpy(
        &bvr_get_instance()->audio.commands[bvr_get_instance()->audio.command_count++], 
        command, sizeof(struct bvr_audio_command_s)
    );
}

void bvr_destroy_audio_mixer(bvr_audio_mixer_t* mixer){
    BVR_ASSERT(mixer);

    SDL_CloseAudioDevice(mixer->device_id);
    mixer->context = NULL;
    mixer->avail = false;
}