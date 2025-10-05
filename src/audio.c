#include <BVR/audio.h>
#include <BVR/config.h>

#include <portaudio.h>

static int bvri_audio_callback(const void* input, void* output,
        unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* time, 
        PaStreamCallbackFlags flags, void* data){
    
    bvr_audio_buffer_t* buffer = (bvr_audio_buffer_t*) data;
    float* outbuffer = (float*)output;

    for(uint32 i = 0; i < framesPerBuffer; i++){
        *outbuffer++ = buffer->left_phase;
        *outbuffer++ = buffer->right_phase;

        buffer->left_phase += 0.3f;
        buffer->right_phase += 0.1f;
        if(buffer->left_phase >= 1.0f) buffer->left_phase -= 2.0f;
        if(buffer->right_phase >= 1.0f) buffer->right_phase -= 2.0f;
    }

    return 0;
}

void bvr_audio_play(bvr_audio_stream_t* stream){
    BVR_ASSERT(stream);
    if(stream->stream){
        Pa_StartStream(stream->stream);
    }
}

void bvr_audio_stop(bvr_audio_stream_t* stream){
    BVR_ASSERT(stream);
    if(stream->stream){
        Pa_StopStream(stream->stream);
    }
}

int bvr_create_audio_stream(bvr_audio_stream_t* stream, int sampleRate, int bufsize){
    BVR_ASSERT(stream);

    BVR_ASSERT(Pa_Initialize() == 0);

    int status;
    PaStreamParameters ouput;
    
    ouput.device = Pa_GetDefaultOutputDevice();
    if(ouput.device == paNoDevice){
        // there is no device
        BVR_ASSERT(0);
    }

    const PaDeviceInfo* info = Pa_GetDeviceInfo(ouput.device);
    BVR_PRINTF("Audio Interface %s", info->name);

    ouput.channelCount = 2;
    ouput.sampleFormat = paFloat32;
    ouput.suggestedLatency = info->defaultLowOutputLatency;
    ouput.hostApiSpecificStreamInfo = NULL;

    status = Pa_OpenStream(&stream->stream, NULL, 
        &ouput, sampleRate, bufsize, paNoFlag, bvri_audio_callback, &stream->buffer
    );
    BVR_ASSERT(status == 0);

    stream->channels = 2;
    stream->sample_rate = sampleRate;
    stream->frame_per_buffer = bufsize;
}

void bvr_destroy_audio_stream(bvr_audio_stream_t* stream){
    Pa_CloseStream(&stream->stream);
    Pa_Terminate();

    stream->stream = NULL;
}