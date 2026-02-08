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
    uint32 format;

    uint32 bloc_format;
    uint32 bloc_size;
    uint16 audio_format;
    uint16 channel_count;
    uint32 sample_rate;
    uint32 byte_per_sec;
    uint16 byte_per_bloc;
    uint16 bits_per_sample;
};

static int bvri_load_wav(bvr_audio_t* audio, FILE* file){
    BVR_ASSERT(audio);
    BVR_ASSERT(file);

    // check for audio stream
    bvr_audio_stream_t* stream = &bvr_get_instance()->audio;
    BVR_ASSERT(stream->avail);

    uint8* wave_bytes = NULL;
    struct bvri_wavheader_s header;
    uint32 packed_bytes, unpacked_bytes;

    fseek(file, 0, SEEK_SET);
    fread(&header, sizeof(struct bvri_wavheader_s), 1, file);

    bvr_fread32_le(file); // skip data bloc id;
    packed_bytes = bvr_fread32_le(file);

    audio->wave = malloc(packed_bytes);
    audio->wave_length = packed_bytes;
    audio->channels = header.channel_count;
    audio->sample_rate = header.sample_rate;
    audio->byte_per_sample = header.bits_per_sample / 8;

    switch (header.bits_per_sample)
    {
    case 8:  audio->format = BVR_AUDIO_INT8; break;
    case 16: audio->format = BVR_AUDIO_INT16; break;
    case 32: audio->format = BVR_AUDIO_INT32; break;
    
    default:
        audio->format = BVR_AUDIO_INT8;
        break;
    }

    // copy raw wav to data
    unpacked_bytes = fread(audio->wave, sizeof(uint8), packed_bytes, file);
    BVR_ASSERT(packed_bytes == unpacked_bytes);

    return BVR_TRUE;
}

#endif

static void bvri_quantize_audio(bvr_audio_t* audio){
    BVR_ASSERT(audio);

    if(!audio->wave){
        return;
    }

    if(audio->format != BVR_AUDIO_INT16){
        BVR_PRINT("shall resize");
    }
}

int bvr_create_audiof(bvr_audio_t* audio, FILE* file){
    BVR_ASSERT(audio);
    BVR_ASSERT(file);

    int status = 0;

    audio->wave = NULL;
    audio->wave_length = 0;
    audio->channels = 0;
    audio->sample_rate = 0;
    audio->byte_per_sample = 0;

#ifndef BVR_NO_WAV
    if(bvri_is_wav(file)){
        status |= bvri_load_wav(audio, file);
    }
#endif

    bvri_quantize_audio(audio);

    return status;
}

void bvr_audio_play(bvr_audio_t* audio){
    BVR_ASSERT(audio);
    BVR_ASSERT(audio->wave);

    struct bvr_audio_command_s cmd;
    cmd.wave = audio->wave;
    cmd.wave_length = audio->wave_length;
    cmd.channels = audio->channels;
    cmd.stride = audio->byte_per_sample * audio->channels;

    bvr_audio_add_wave_command(&cmd);
}

void bvr_destroy_audio(bvr_audio_t* audio){
    BVR_ASSERT(audio);

    free(audio->wave);
    audio->wave = NULL;
}

static void bvri_audio_callback(void* _stream, SDL_AudioStream* sdl, int additional_amount, int total_amount){
    uint32 done = 0;
    bvr_audio_stream_t* stream = (bvr_audio_stream_t*)_stream;

    // clear previous audio buffer
    memset(stream->mixer.buffer, 0, BVR_AUDIO_BYTE_LENGTH * sizeof(short));

    stream->mixer.avail_buffer_length = MIN(additional_amount, BVR_AUDIO_BYTE_LENGTH);
    
    for (size_t i = 0; i < stream->mixer.command_count; i++)
    {
        if (!bvr_audio_do_wave_command(&stream->mixer.commands[i])) {
            stream->mixer.commands[done++] = stream->mixer.commands[i];
        }
    }

    stream->mixer.command_count = done;

    SDL_PutAudioStreamData(stream->context, stream->mixer.buffer, stream->mixer.avail_buffer_length);
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
    stream->mixer.gain = 0.5f;

    stream->context = SDL_OpenAudioDeviceStream(
        BVR_AUDIO_DEFAULT_OUTPUT, &config, 
        bvri_audio_callback, stream
    );
    BVR_ASSERT(stream->context);

    memset(stream->mixer.commands, 0, sizeof(stream->mixer.commands));
    stream->mixer.command_count = 0;
    
    SDL_ResumeAudioStreamDevice(stream->context);

    stream->avail = true;
    return BVR_TRUE;
}

/**
 * Because of how audio works, we might need to follow the same way as real sound mixers
 * https://sound-au.com/articles/audio-mixing.htm
 */
bool bvr_audio_do_wave_command(struct bvr_audio_command_s* command){
    bvr_audio_stream_t* stream = &bvr_get_instance()->audio;

    uint32 byte_count = MIN(command->wave_length, stream->mixer.avail_buffer_length);
    uint32 frame_count = byte_count / command->stride;

    short* src = command->wave;
    short* dst = stream->mixer.buffer;

    for (uint32 f = 0; f < frame_count; f++)
    {
        if (command->channels == 1)
        {
            short mono = (dst[2 * f] + src[f]) / stream->mixer.command_count;

            dst[2 * f + 0] = mono;
            dst[2 * f + 1] = mono;
        }
        else
        {
            dst[2 * f + 0] = (dst[2 * f] + src[2 * f]) / stream->mixer.command_count;
            dst[2 * f + 1] = (dst[2 * f + 1] + src[2 * f + 1]) / stream->mixer.command_count;
        }
    }

    command->wave = command->wave + (size_t)byte_count;
    command->wave_length -= byte_count;

    return command->wave_length == 0;
}

void bvr_audio_add_wave_command(const struct bvr_audio_command_s* command){
    BVR_ASSERT(command);

    // when there is no initialized instance
    if(!bvr_get_instance()->audio.avail){
        return;
    }

    // when there is no space for a new command
    if(bvr_get_instance()->audio.mixer.command_count + 1 >= BVR_MAX_AUDIO_COMMAND){
        return;
    }

    // try to find an available command storing space
    memcpy(
        &bvr_get_instance()->audio.mixer.commands[bvr_get_instance()->audio.mixer.command_count++], 
        command, sizeof(struct bvr_audio_command_s)
    );
}

void bvr_destroy_audio_stream(bvr_audio_stream_t* stream){
    BVR_ASSERT(stream);

    SDL_CloseAudioDevice(stream->device_id);
    stream->context = NULL;
    stream->avail = false;
}