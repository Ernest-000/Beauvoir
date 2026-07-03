#include <bvr/audio.h>
#include <bvr/book.h>
#include <bvr/io.h>
#include <bvr/window.h>

// #include <SDL3/SDL_audio.h>

#include <stdlib.h>
#include <memory.h>

#ifdef _WIN32
#elif __unix__
    #include <spa/param/audio/format-utils.h>
    #include <pipewire/pipewire.h>
#else
    #error unable to find a supported audio backend.
#endif

#define BVR_AUDIO_FORMAT BVR_AUDIO_FORMAT_INT16
#define BVR_AUDIO_NO_SIGNAL (0)

// audio implementations promises
void bvri_create_mixer_impl(bvr_audio_mixer_t* mixer);
void bvri_write_audio_stream_impl(void* userdata);
void bvri_destroy_mixer_impl(bvr_audio_mixer_t* mixer);

// callbacks
// static void bvri_audio_callback(void* _stream, int additional_amount, int total_amount);
static struct bvr_audio_command_s* bvri_audio_find_audio_command(uint32 id);

#pragma region AUDIO_SOURCE

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
static int bvri_load_wav(FILE* file, bvr_audio_t* audio){
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

    audio->wave = NULL;
    audio->wave_length = 0;

    audio->channels = fmt.channel_count;
    audio->sample_rate = fmt.sample_rate;
    audio->sample_depth = fmt.bits_per_sample / 8;
    audio->duration = chunk.size / audio->sample_depth;

    switch (fmt.bits_per_sample)
    {
    case 8:  audio->format = BVR_AUDIO_FORMAT_INT8; break;
    case 16: audio->format = BVR_AUDIO_FORMAT_INT16; break;
    case 24: audio->format = BVR_AUDIO_FORMAT_INT24; break;
    case 32: audio->format = BVR_AUDIO_FORMAT_INT32; break;
    
    default:
        audio->format = BVR_AUDIO_FORMAT_INT8;
        break;
    }

    if(audio->format == BVR_AUDIO_FORMAT_INT24){
        // because it's hard to store 24bit audio
        // we need to read them as 32bit 
        uint32* wave = malloc(chunk.size / audio->sample_depth * sizeof(int32));

        audio->wave = (short*)wave;
        audio->wave_length = chunk.size / audio->sample_depth * sizeof(int32);

        for(int f = 0; f < chunk.size / audio->sample_depth; f++)
        {
            wave[f] = bvr_fread24_le(file);
        }
        
        // make shure to overwrite values to fake 32bit
        audio->format = BVR_AUDIO_FORMAT_INT32;
        audio->sample_depth = 4;
    }
    else {
        // just copy raw data
        audio->wave = malloc(chunk.size);
        audio->wave_length = chunk.size;

        unpacked_bytes = fread(audio->wave, sizeof(uint8), chunk.size, file);
        BVR_ASSERT(chunk.size == unpacked_bytes);
    }
    
    return audio->wave && audio->wave_length;
}

#endif

#ifndef BVR_NO_VORBIS

#define BVRI_OGG_SEGMENT_SIZE 255
#define BVRI_OGG_DATA_SIZE 65025

#define BVRI_VORBIS_CODEBOOK_MAX 128
#define BVRI_VORBIS_FLOOR_MAX 64
#define BVRI_VORBIS_RESIDUE_MAX 64
#define BVRI_VORBIS_MAPPING_MAX 64
#define BVRI_VORBIS_MODE_MAX 64
#define BVRI_VORBIS_CHANNEL_MAX 8

#define BVRI_VORBIS_UNUSED 0xFF

#define BVRI_OGG_CP 0x01 // Continued package
#define BVRI_OGG_BOS 0x02 // First page of the bitstream
#define BVRI_OGG_EOF 0x04 // End of stream

// Check if it is an ogg container
static int bvri_is_ogg(FILE* file){
    uint8 sig[4];
    fseek(file, 0, SEEK_SET);
    fread(sig, sizeof(uint8), 4, file);

    bvr_freadu8_le(file); // version

    // get the header type
    // 0x02 meaning it's the begining of the stream
    uint8 type = bvr_freadu8_le(file);

    return BVR_STRNCMP(sig, "OggS", 4) && BVR_HAS_FLAG(type, BVRI_OGG_BOS);
}

struct bvri_oggpage_s {
    uint8 header_type;
    int64 absolute_position;
    int serial_number;
    int page_number;

    uint8 lacing[BVRI_OGG_SEGMENT_SIZE];
    uint8 lacing_count;

    uint8 data[BVRI_OGG_DATA_SIZE];
    uint32 data_size;
    
    uint64 lacing_cursor;
    uint64 data_cursor;

    bool continued;
    bool eof;
};

struct bvri_vorbis_s {
    uint32 version;

    uint8 channels;
    uint32 sample_rate;
    uint32 bitrate_max;
    uint32 bitrate_nominal;
    uint16 bitrate_min;
    uint16 blocksize_0;
    uint32 blocksize_1;

    uint8 codebook_count;
    uint8 floor_count;
    uint8 residue_count;
    uint8 mapping_count;
    uint8 mode_count;

    struct bvri_vorbis_codebook_s {
        uint16 dimension;
        uint32 entries;
        uint8 ordered;
        uint8 sparsed;

        uint8 lookup_type;
        uint32 lookup_values;

        uint32* lenghts;
        float* vqs;
    } codebooks[BVRI_VORBIS_CODEBOOK_MAX];

    union bvri_vorbis_floor_s{
        struct {
            uint8 floor_type;

            uint8 order;
            uint16 rate;
            uint16 bark_map_size;
            uint16 amplitude_bits;
            uint16 amplitude_offset;
            uint8 book_count;
            uint8 books[16];
        } lsp;

        struct {
            uint8 floor_type;

            uint8 partition_count;
            uint8 partitions[32];

            uint8 class_dimensions[16];
            uint8 class_subclasses[16];
            uint8 class_masterbooks[16];
            uint8 class_books[16][8];

            uint8 floor_multiplier;
            uint8 floor_rangebits;

            uint16 x_points[64];
            uint16 neighbors[64][2];
            uint16 x_count;
        } piecewire
    } floors[BVRI_VORBIS_FLOOR_MAX];

    struct bvri_vorbis_residue_s {
        uint8 residue_type;
        uint32 begin;
        uint32 end;
        uint32 partition_size;
        uint8 classification;
        uint8 class_book;

        uint8 cascade[64];
        uint8 books[64][8];
    } residues[BVRI_VORBIS_RESIDUE_MAX];

    struct bvri_vorbis_mapping_s {
        uint8 mapping_submaps;
        uint8 mapping_steps;
        uint32 magnitudes[256];
        uint32 angles[256];
        uint32 muxs[BVRI_VORBIS_CHANNEL_MAX]; // max two audio channels
        uint32 floors[16];
        uint32 residues[16];
    } mapping[BVRI_VORBIS_MAPPING_MAX];

    struct bvri_vorbis_mode_s {
        uint8 block_flag;
        uint16 window_type;
        uint16 transform_type;
        uint8 mapping;
    } modes[BVRI_VORBIS_MODE_MAX]
};

struct bvri_vorbis_bitread_s {
    uint8* buffer;
    uint64 size;
    uint64 byte_cursor;
    int bit_cursor;
};

/**
 * https://xiph.org/ogg/doc/framing.html
 */
static uint64 bvri_oggreadpage(FILE* file, struct bvri_oggpage_s* page){
    BVR_ASSERT(page);
    
    uint8 sig[4];
    uint64 readed_bytes = 0;

    // read sig
    fread(sig, sizeof(uint8), 4, file);
    BVR_ASSERT(BVR_STRNCMP(sig, "OggS", 4));

    // version
    bvr_freadu8_le(file);

    page->header_type = bvr_freadu8_le(file);
    page->absolute_position = bvr_fread64_le(file);
    page->serial_number = bvr_fread32_le(file);
    page->page_number = bvr_fread32_le(file);

    // checksum
    bvr_fread32_le(file);

    // segment table
    page->data_size = 0;
    page->lacing_cursor = 0;
    page->data_cursor = 0;
    page->lacing_count = bvr_freadu8_le(file);
    for (size_t i = 0; i < page->lacing_count; i++)
    {
        page->lacing[i] = bvr_freadu8_le(file);
        page->data_size += page->lacing[i];
    }

    page->continued = BVR_HAS_FLAG(page->header_type, BVRI_OGG_CP);
    page->eof = BVR_HAS_FLAG(page->header_type, BVRI_OGG_EOF);
    
    readed_bytes = fread(page->data, page->data_size, sizeof(uint8), file);
    return readed_bytes > 0;
}

/**
 * https://www.xiph.org/ogg/doc/framing.html
 */
static int bvri_oggnext(FILE* file, struct bvri_oggpage_s* page, struct bvr_buffer_s* packet){
    BVR_ASSERT(file);
    BVR_ASSERT(page);
    BVR_ASSERT(packet);

    uint64 packet_offset = 0;

    // make shure to have empty buffers
    if(packet->size){    
        free(packet->data);

        packet->data = NULL;
        packet->size = 0; // avail size
        packet->elemsize = 0; // used size
    }

    while (!page->eof)
    {
        if(page->lacing_cursor >= page->lacing_count){
            if(!bvri_oggreadpage(file, page)){
                // when failed to read file
                return BVR_FALSE; 
            }
        }

        while (page->lacing_cursor < page->lacing_count)
        {
            uint8 lz = page->lacing[page->lacing_cursor++];

            packet->elemsize += lz;

            if(packet->size < packet->elemsize){
                BVR_BUFFER_REALLOC((*packet), page->data_size);
            }

            memcpy(packet->data + packet_offset, page->data + page->data_cursor, lz);
            packet_offset += lz;
            page->data_cursor += lz;

            if(lz < 255){
                return BVR_TRUE;
            }
        }
    }
    
    return BVR_FALSE;
}

static int bvri_is_vorbis(FILE* file){
    fseek(file, 0, SEEK_SET);

    uint8 sig[6];
    struct bvri_oggpage_s page;
    bvri_oggreadpage(file, &page);
    
    // Check if this is the start of the ogg file
    if(page.page_number != 0 && !BVR_HAS_FLAG(page.header_type, BVRI_OGG_BOS)){
        return BVR_FALSE;
    }

    // set signature
    memcpy(sig, &page.data[1], sizeof(sig));

    // check for signature
    return BVR_STRNCMP(sig, "vorbis", 6);
}

/**
 * Returns the smallest number of bits to represent a number
 */
static int bvri_vorbis_ilog(int value){
    int n = 0;
    while (value > 0)
    {
        n++;
        value >>= 1;
    }

    return n;
}

/**
 * Integer comparative function
 */
static int bvri_vorbis_compfunc(const void* a, const void* b) {
    uint16* x = (uint16*) a;
    uint16* y = (uint16*) b;
    return *x - *y;
}


/**
 * Vorbis formatted float casting
 */
static float bvri_vorbis_castf32(int a){
    int mantissa = a & 0x1FFFFF;
    int exponent = (a >> 21) & 0x3FF;
    int sign = (a >> 31) & 1;
    float value = mantissa * (powf(2.0f, (exponent - 788)));
    return sign ? -value : value;
}

/**
 * Return scalar length of a lookup table of 1
 * ?????
 */
static int bvri_vorbis_lookup1(struct bvri_vorbis_codebook_s* codebook){
    int rounded = (int)powf(codebook->entries, (1.0f / codebook->dimension));
    while (powf(rounded + 1, codebook->dimension) < codebook->entries)
    {
        rounded++;
    }

    return rounded;
}

/**
 * Small bit reader function
 */
static uint32 bvri_vorbis_readbit(struct bvri_vorbis_bitread_s* reader, const int bit_count)
{
    uint32 result = 0;
    int i;

    for (i = 0; i < bit_count; i++) {
        //if (reader->byte_cursor >= reader->size){
        //    BVR_PRINTF("%i > %i", reader->byte_cursor, reader->size);
        //    return result;
        //}
        //BVR_ASSERT(reader->byte_cursor < reader->size);

        result |= ((*reader->buffer >> reader->bit_cursor) & 1) << i;

        if (++reader->bit_cursor == 8) {
            reader->bit_cursor = 0;
            reader->byte_cursor++;
            reader->buffer++;
        }
    }

    return result;
}

/**
 * https://www.xiph.org/vorbis/doc/Vorbis_I_spec.pdf
 */
static void bvri_vorbis_decodeheader(struct bvri_vorbis_s *vorbis, const uint8* c_packet, const uint64 packet_size)
{
    BVR_ASSERT(vorbis);
    BVR_ASSERT(c_packet);
    BVR_ASSERT(packet_size);

    struct bvri_vorbis_bitread_s stream;
    stream.buffer = c_packet;
    stream.size = packet_size;
    stream.byte_cursor = 0;
    stream.bit_cursor = 0;

    switch (bvr_mread8_le(&stream.buffer))
    {
    case 0x1:
        // align packet pointer
        stream.buffer = &c_packet[7];

        /* information packet */
        vorbis->version = bvr_mread32_le(&stream.buffer);
        vorbis->channels = bvr_mread8_le(&stream.buffer);
        vorbis->sample_rate = bvr_mread32_le(&stream.buffer);
        vorbis->bitrate_max = bvr_mread32_le(&stream.buffer);
        vorbis->bitrate_nominal = bvr_mread32_le(&stream.buffer);
        vorbis->bitrate_min = bvr_mread32_le(&stream.buffer);
        vorbis->blocksize_0 = pow(2, bvri_vorbis_readbit(&stream, 4));
        vorbis->blocksize_1 = pow(2, bvri_vorbis_readbit(&stream, 4));

        BVR_ASSERT(bvri_vorbis_readbit(&stream, 1));
        break;

    case 0x3:
        /* comment packet do nothing for now */
        break;

    case 0x5:
        /* setup packet */

        // align packet pointer
        stream.buffer = &c_packet[7];

        // decode codebooks
        {    
            uint32 codebook_sig;

            vorbis->codebook_count = bvri_vorbis_readbit(&stream, 8) + 1;
            BVR_ASSERT(vorbis->codebook_count < BVRI_VORBIS_CODEBOOK_MAX);

            for (size_t i = 0; i < vorbis->codebook_count; i++)
            {
                // check for codebook signature
                codebook_sig = bvri_vorbis_readbit(&stream, 24);

                // not a valid codebook maybe next packet :D
                if(codebook_sig != 0x564342){
                    break;
                }

                vorbis->codebooks[i].dimension = bvri_vorbis_readbit(&stream, 16);
                vorbis->codebooks[i].entries = bvri_vorbis_readbit(&stream, 24);
                vorbis->codebooks[i].ordered = bvri_vorbis_readbit(&stream, 1);

                vorbis->codebooks[i].lenghts = malloc(vorbis->codebooks[i].entries * sizeof(uint32));
                vorbis->codebooks[i].vqs = malloc(vorbis->codebooks[i].entries * vorbis->codebooks[i].dimension * sizeof(uint32));
                BVR_ASSERT(vorbis->codebooks[i].lenghts);
                BVR_ASSERT(vorbis->codebooks[i].vqs);

                // if is ordered
                if(vorbis->codebooks[i].ordered){
                    uint32 current_entry = 0;
                    uint32 current_length = bvri_vorbis_readbit(&stream, 5) + 1;
                    uint32 bit_count = 0;
                    uint32 packed_lengths = 0;

                    while (current_entry < vorbis->codebooks[i].entries)
                    {
                        bit_count = bvri_vorbis_ilog(vorbis->codebooks[i].entries - current_entry);
                        packed_lengths = bvri_vorbis_readbit(&stream, bit_count);

                        for (size_t p = 0; p < packed_lengths; p++)
                        {
                            vorbis->codebooks[i].lenghts[current_entry++] = current_length;  
                        }
                        
                        current_length++;
                    }
                }
                else {
                    uint32 current_entry = 0;

                    vorbis->codebooks[i].sparsed = bvri_vorbis_readbit(&stream, 1);

                    for (size_t y = 0; y < vorbis->codebooks[i].entries; y++)
                    {
                        uint32 current_length = 0; 
                        
                        if(vorbis->codebooks[i].sparsed){
                            uint8 flag = bvri_vorbis_readbit(&stream, 1);
                            
                            // check for flag
                            if(flag){
                                current_length = bvri_vorbis_readbit(&stream, 5) + 1;
                            }   
                            else {
                                current_length = BVRI_VORBIS_UNUSED;
                            }                         
                        }
                        else {
                            current_length = bvri_vorbis_readbit(&stream, 5) + 1;
                        }
                        
                        vorbis->codebooks[i].lenghts[current_entry++] = current_length;
                    }
                    
                }

                vorbis->codebooks[i].lookup_type = bvri_vorbis_readbit(&stream, 4);
                BVR_ASSERT(vorbis->codebooks[i].lookup_type <= 2);

                if(vorbis->codebooks[i].lookup_type > 0) {
                    uint32* unpacked_vectors = NULL;
                    float minimum = bvri_vorbis_castf32(bvri_vorbis_readbit(&stream, 32));
                    float delta = bvri_vorbis_castf32(bvri_vorbis_readbit(&stream, 32));
                    uint8 bits = bvri_vorbis_readbit(&stream, 4) + 1;
                    uint8 seq = bvri_vorbis_readbit(&stream, 1);

                    if(vorbis->codebooks[i].lookup_type == 1){
                        // type 1
                        vorbis->codebooks[i].lookup_values = bvri_vorbis_lookup1(&vorbis->codebooks[i]);
                    }
                    else {
                        // type 2
                        vorbis->codebooks[i].lookup_values = vorbis->codebooks[i].entries * vorbis->codebooks[i].dimension;
                    }

                    unpacked_vectors = malloc(vorbis->codebooks[i].lookup_values * sizeof(uint32));
                    BVR_ASSERT(unpacked_vectors);

                    // read each unpacked vectors from stream
                    for (size_t y = 0; y < vorbis->codebooks[i].lookup_values; y++)
                    {
                        unpacked_vectors[y] = bvri_vorbis_readbit(&stream, bits);
                    }
                    
                    for (size_t e = 0; e < vorbis->codebooks[i].entries; e++)
                    {
                        float last = 0.0f;
                        float value = 0.0f;
                        uint32 index;
                        uint32 multiplicand_offset = 0;

                        if(vorbis->codebooks[i].lookup_type == 1){
                            // type 1
                            int index_divisor = 1;
                            int lookup = bvri_vorbis_lookup1(&vorbis->codebooks[i]);

                            for (size_t d = 0; d < vorbis->codebooks[i].dimension; d++)
                            {
                                index = e * vorbis->codebooks[i].dimension + d;

                                multiplicand_offset = e / index_divisor % lookup;
                                value = unpacked_vectors[multiplicand_offset] * delta + minimum + last;

                                vorbis->codebooks[i].vqs[index] = value;
                                if(seq){
                                    last = value;
                                }

                                index_divisor *= lookup;
                            }
                            
                        }
                        else {
                            // type 2
                            multiplicand_offset = e * vorbis->codebooks[i].dimension;

                            for (size_t d = 0; d < vorbis->codebooks[i].dimension; d++)
                            {
                                index = e * vorbis->codebooks[i].dimension + d;
                                value = unpacked_vectors[multiplicand_offset + d] * delta + minimum + last;

                                vorbis->codebooks[i].vqs[index] = value;
                                if(seq){
                                    last = value;
                                }
                            }
                            
                        }
                    }
                    
                    // BVR_PRINTF("minimum %f delta %f bits %i", minimum, delta, bits);

                    free(unpacked_vectors);
                }
                else {
                    // no-op
                }

                // BVR_PRINTF("dimension %i entries %i order %i", vorbis->codebooks[i].dimension, vorbis->codebooks[i].entries, vorbis->codebooks[i].ordered);
            }
        }

        // check for time domain
        {
            uint16 time_count = bvri_vorbis_readbit(&stream, 6) + 1;
            for (size_t i = 0; i < time_count; i++)
            {
                BVR_ASSERT(bvri_vorbis_readbit(&stream, 16) == 0);
            }
        }

        // decode floors
        {
            vorbis->floor_count = bvri_vorbis_readbit(&stream, 6) + 1;
            BVR_ASSERT(vorbis->floor_count < BVRI_VORBIS_FLOOR_MAX);

            for (size_t i = 0; i < vorbis->floor_count; i++)
            {
                vorbis->floors[i].lsp.floor_type = bvri_vorbis_readbit(&stream, 16);
                BVR_ASSERT(vorbis->floors[i].lsp.floor_type <= 1);
                
                if(vorbis->floors[i].lsp.floor_type == 0){
                    // support old floor type (Line Spectral Pair)
                    // may not support?
                    vorbis->floors[i].lsp.order = bvri_vorbis_readbit(&stream, 8);
                    vorbis->floors[i].lsp.rate = bvri_vorbis_readbit(&stream, 16);
                    vorbis->floors[i].lsp.bark_map_size = bvri_vorbis_readbit(&stream, 16);
                    vorbis->floors[i].lsp.amplitude_bits = bvri_vorbis_readbit(&stream, 6);
                    vorbis->floors[i].lsp.amplitude_offset = bvri_vorbis_readbit(&stream, 8);
                    vorbis->floors[i].lsp.book_count = bvri_vorbis_readbit(&stream, 4) + 1;

                    for (size_t b = 0; b < vorbis->floors[i].lsp.book_count; b++)
                    {
                        vorbis->floors[i].lsp.books[b] = bvri_vorbis_readbit(&stream, 8);
                    }
                }
                else {
                    // support for piecewire straight line
                    int max_partition = -1;

                    vorbis->floors[i].piecewire.partition_count = bvri_vorbis_readbit(&stream, 5);
                    for (size_t p = 0; p < vorbis->floors[i].piecewire.partition_count; p++)
                    {
                        vorbis->floors[i].piecewire.partitions[p] = bvri_vorbis_readbit(&stream, 4);
                        max_partition = MAX(max_partition, vorbis->floors[i].piecewire.partitions[p]);
                    }

                    for (size_t m = 0; m <= max_partition; m++)
                    {
                        vorbis->floors[i].piecewire.class_dimensions[m] = bvri_vorbis_readbit(&stream, 3) + 1;
                        vorbis->floors[i].piecewire.class_subclasses[m] = bvri_vorbis_readbit(&stream, 2);
                        vorbis->floors[i].piecewire.class_masterbooks[m] = 0;

                        if(vorbis->floors[i].piecewire.class_subclasses[m]){
                            vorbis->floors[i].piecewire.class_masterbooks[m] = bvri_vorbis_readbit(&stream, 8);
                        }

                        for (size_t j = 0; j < pow(2.0, vorbis->floors[i].piecewire.class_subclasses[m]); j++)
                        {
                            vorbis->floors[i].piecewire.class_books[m][j] = bvri_vorbis_readbit(&stream, 8) - 1;
                        }
                    }

                    vorbis->floors[i].piecewire.floor_multiplier = bvri_vorbis_readbit(&stream, 2) + 1;
                    vorbis->floors[i].piecewire.floor_rangebits = bvri_vorbis_readbit(&stream, 4);

                    vorbis->floors[i].piecewire.x_points[0] = 0;
                    vorbis->floors[i].piecewire.x_points[1] = pow(2, vorbis->floors[i].piecewire.floor_rangebits);
                    vorbis->floors[i].piecewire.x_count = 2;

                    uint32 class = 0;
                    for (size_t p = 0; p < vorbis->floors[i].piecewire.partition_count; p++)
                    {
                        class = vorbis->floors[i].piecewire.partitions[p];

                        for (size_t j = 0; j < vorbis->floors[i].piecewire.class_dimensions[class]; j++)
                        {
                            vorbis->floors[i].piecewire.x_points[vorbis->floors[i].piecewire.x_count++] 
                                = bvri_vorbis_readbit(&stream, vorbis->floors[i].piecewire.floor_rangebits);
                        }
                    }

                    // sort x points
                    qsort(
                        vorbis->floors[i].piecewire.x_points, 
                        vorbis->floors[i].piecewire.x_count,
                        sizeof(uint16), bvri_vorbis_compfunc
                    );

                    // calculate neighbors
                    vorbis->floors[i].piecewire.neighbors[0][0] = 0;
                    vorbis->floors[i].piecewire.neighbors[0][1] = 0;
                    vorbis->floors[i].piecewire.neighbors[1][0] = 0;
                    vorbis->floors[i].piecewire.neighbors[1][1] = 0;

                    for (size_t n = 0; n < vorbis->floors[i].piecewire.x_count; n++)
                    {
                        uint16 low_neighbor = 0; 
                        uint16 high_neighbor = BVR_UINT16_MAX; 
                        int low_index = 0;
                        int high_index = 0;

                        for (size_t m = 0; m < n; m++)
                        {
                            if(vorbis->floors[i].piecewire.x_points[m] > low_neighbor && 
                                vorbis->floors[i].piecewire.x_points[m] < vorbis->floors[i].piecewire.x_points[n]){

                                low_neighbor = vorbis->floors[i].piecewire.x_points[m];
                                low_index = m;
                            }
                            
                            if(vorbis->floors[i].piecewire.x_points[m] < high_neighbor &&
                                vorbis->floors[i].piecewire.x_points[m] > vorbis->floors[i].piecewire.x_points[n]){

                                high_neighbor = vorbis->floors[i].piecewire.x_points[m];
                                high_index = m;
                            }
                        }
                        
                        vorbis->floors[i].piecewire.neighbors[n][0] = low_index;
                        vorbis->floors[i].piecewire.neighbors[n][1] = high_index;
                    }

                    // BVR_PRINTF("partition count %i, point count %i", vorbis->floors[i].piecewire.partition_count, vorbis->floors[i].piecewire.x_count);                
                }
            }
        }

        // read residues
        {
            vorbis->residue_count = bvri_vorbis_readbit(&stream, 6) + 1;
            BVR_ASSERT(vorbis->residue_count < BVRI_VORBIS_RESIDUE_MAX);

            for (size_t i = 0; i < vorbis->residue_count; i++)
            {
                vorbis->residues[i].residue_type = bvri_vorbis_readbit(&stream, 16);
                BVR_ASSERT(vorbis->residues[i].residue_type <= 2);

                vorbis->residues[i].begin = bvri_vorbis_readbit(&stream, 24);
                vorbis->residues[i].end = bvri_vorbis_readbit(&stream, 24);
                vorbis->residues[i].partition_size = bvri_vorbis_readbit(&stream, 24) + 1;
                vorbis->residues[i].classification = bvri_vorbis_readbit(&stream, 6) + 1;
                vorbis->residues[i].class_book = bvri_vorbis_readbit(&stream, 8);

                for (size_t c = 0; c < vorbis->residues[i].classification; c++)
                {
                    uint8 high_bit = 0;
                    uint8 low_bit = bvri_vorbis_readbit(&stream, 3);

                    if(bvri_vorbis_readbit(&stream, 1)){
                        high_bit = bvri_vorbis_readbit(&stream, 5);
                    }

                    vorbis->residues[i].cascade[c] = high_bit * 8 + low_bit;
                }

                for (size_t c = 0; c < vorbis->residues[i].classification; c++)
                {
                    for (size_t b = 0; b < 8; b++)
                    {
                        vorbis->residues[i].books[c][b] = BVRI_VORBIS_UNUSED;
                        if(vorbis->residues[i].cascade[c] & (1 << b)){
                            vorbis->residues[i].books[c][b] = bvri_vorbis_readbit(&stream, 8);
                        }
                    }
                }                
            }
        }

        // read mappings
        {
            vorbis->mapping_count = bvri_vorbis_readbit(&stream, 6) + 1;
            BVR_ASSERT(vorbis->channels); // check for audio channels
            BVR_ASSERT(vorbis->mapping_count < BVRI_VORBIS_MAPPING_MAX);

            for (size_t i = 0; i < vorbis->mapping_count; i++)
            {
                uint16 mapping_type = bvri_vorbis_readbit(&stream, 16);
                BVR_ASSERT(mapping_type == 0);

                vorbis->mapping[i].mapping_submaps = 1;
                if(bvri_vorbis_readbit(&stream, 1)){
                    vorbis->mapping[i].mapping_submaps = bvri_vorbis_readbit(&stream, 4) + 1;
                }

                vorbis->mapping[i].mapping_steps = 0;
                if(bvri_vorbis_readbit(&stream, 1)){
                    // polar channel mapping

                    vorbis->mapping[i].mapping_steps = bvri_vorbis_readbit(&stream, 8) + 1;
                    for (size_t j = 0; j < vorbis->mapping[i].mapping_steps; j++)
                    {
                        vorbis->mapping[i].magnitudes[j] = bvri_vorbis_readbit(&stream, bvri_vorbis_ilog(vorbis->channels - 1));    
                        vorbis->mapping[i].angles[j] = bvri_vorbis_readbit(&stream, bvri_vorbis_ilog(vorbis->channels - 1));    
                    }
                }

                bvri_vorbis_readbit(&stream, 2); // unused fields
                if(vorbis->mapping[i].mapping_submaps > 1){
                    for (size_t j = 0; j < vorbis->channels; j++)
                    {
                        if(j < BVRI_VORBIS_CHANNEL_MAX){
                            vorbis->mapping[i].muxs[j] = bvri_vorbis_readbit(&stream, 4);
                            BVR_ASSERT(vorbis->mapping[i].muxs[j] > vorbis->mapping[i].mapping_submaps);
                        }
                        else {
                            // skip to avoid channel overflow
                            bvri_vorbis_readbit(&stream, 4);
                        }
                    }
                }

                for (size_t j = 0; j < vorbis->mapping[i].mapping_submaps; j++)
                {
                    bvri_vorbis_readbit(&stream, 8); // discard
                    vorbis->mapping[i].floors[j] = bvri_vorbis_readbit(&stream, 8);
                    vorbis->mapping[i].residues[j] = bvri_vorbis_readbit(&stream, 8);

                    BVR_ASSERT(vorbis->mapping[i].floors[j] < vorbis->floor_count);
                    BVR_ASSERT(vorbis->mapping[i].residues[j] < vorbis->residue_count);
                }
            }
        }

        // read modes
        {
            vorbis->mode_count = bvri_vorbis_readbit(&stream, 6) + 1;
            BVR_ASSERT(vorbis->mode_count < BVRI_VORBIS_MODE_MAX);

            for (size_t i = 0; i < vorbis->mode_count; i++)
            {
                vorbis->modes[i].block_flag = bvri_vorbis_readbit(&stream, 1);
                vorbis->modes[i].window_type = bvri_vorbis_readbit(&stream, 16);
                vorbis->modes[i].transform_type = bvri_vorbis_readbit(&stream, 16);
                vorbis->modes[i].mapping = bvri_vorbis_readbit(&stream, 8);

                BVR_ASSERT(vorbis->modes[i].mapping < vorbis->mapping_count);
            }
            
        }
        
        // framing flag
        BVR_ASSERT(bvri_vorbis_readbit(&stream, 1));
        
        break;

    default:
        BVR_PRINT("invalid packet type!");
        break;
    }
}

static int bvri_vorbis_decodeaudio(struct bvri_vorbis_s* vorbis, const uint8* c_packet, const uint64 packet_size){
    BVR_ASSERT(vorbis);
    BVR_ASSERT(c_packet);
    BVR_ASSERT(packet_size);

    struct bvri_vorbis_bitread_s stream;
    stream.buffer = c_packet;
    stream.size = packet_size;
    stream.byte_cursor = 0;
    stream.bit_cursor = 0;

    struct {
        uint8 mode;
        uint8 mapping;
    } packet;

    struct {
        uint16 blocksize;

        uint8 prev_window_flag;
        uint8 new_window_flag;

        uint32 window_center;
        uint32 left_win_start;
        uint32 left_win_end;
        uint32 left_win_n;
        
        uint32 right_win_start;
        uint32 right_win_end;
        uint32 right_win_n;

        // maybe use malloc instead?
        float window[BVR_UINT16_MAX];
    } window;

    // check for audio packet
    BVR_ASSERT(bvri_vorbis_readbit(&stream, 1) == 0);

    packet.mode = bvri_vorbis_readbit(&stream, bvri_vorbis_ilog(vorbis->mode_count - 1));
    BVR_ASSERT(packet.mode < vorbis->mode_count);

    packet.mapping = vorbis->modes[packet.mode].mapping;
    BVR_ASSERT(packet.mapping < vorbis->mapping_count);

    window.blocksize = vorbis->blocksize_1;
    
    if(vorbis->modes[packet.mode].block_flag){
        window.blocksize = vorbis->blocksize_0;
    }

    window.prev_window_flag = bvri_vorbis_readbit(&stream, 1);
    window.new_window_flag = bvri_vorbis_readbit(&stream, 1);
    window.window_center = window.blocksize / 2;

    // left side is a hybrid window for laping with a short block 
    if(!window.prev_window_flag && vorbis->modes[packet.mode].block_flag){    
        window.left_win_start = window.blocksize / 4 - vorbis->blocksize_0 / 4;
        window.left_win_end = window.blocksize / 4 + vorbis->blocksize_0 / 4;
        window.left_win_n = vorbis->blocksize_0 / 2;
    }
    else {
        // left half win have normal long shape
        window.left_win_start = 0;
        window.left_win_end = window.window_center;
        window.left_win_n = vorbis->blocksize_0 / 2;
    }

    // right side is a hybrid win for laping with a short block
    if(!window.new_window_flag && vorbis->modes[packet.mode].block_flag){
        window.right_win_start = window.blocksize * (3/4) - vorbis->blocksize_0 / 4;
        window.right_win_end = window.blocksize * (3/4) + vorbis->blocksize_0 / 4;
        window.right_win_n = window.blocksize / 2;
    }
    else {
        // right half win have normal long shape
        window.right_win_start = window.window_center;
        window.right_win_end = window.blocksize;
        window.right_win_n = vorbis->blocksize_0 / 2;
    }

    // set all to zero
    memset(window.window, 0, sizeof(float) * window.blocksize);

    // create left side window
    for (size_t i = window.left_win_start - 1; i < window.left_win_end; i++)
    {
        float offset = i - window.left_win_start + 0.5f;
        window.window[i] = sinf(BVR_HALF_PI * sqrtf(sinf(offset / window.left_win_n * BVR_HALF_PI)));
    }
    
    // create right side window
    for (size_t i = window.left_win_end - 1; i < window.right_win_end; i++)
    {
        float offset = i - window.right_win_start + 0.5f;
        window.window[i] = sinf(BVR_HALF_PI * sqrtf(sinf(offset / window.right_win_n * BVR_HALF_PI + BVR_HALF_PI)));
    }
    
    for (size_t channel = 0; channel < vorbis->channels; channel++)
    {
        union bvri_vorbis_floor_s* floor;

        uint16 submap = vorbis->mapping[packet.mapping].muxs[channel];
        floor = &vorbis->floors[vorbis->mapping[packet.mapping].floors[submap]];
        
        if(floor->lsp.floor_type == 0){
            // do lsp packet decode
            BVR_ASSERT(0 || "invalid packet type");
        }
        else {
            // do piecewire packet decode
            
        }
    }
    
}

/**
 * https://xiph.org/vorbis/doc/Vorbis_I_spec.pdf
 */
static int bvri_load_vorbis(FILE* file, bvr_audio_t* audio){
    BVR_ASSERT(file);
    BVR_ASSERT(audio);

    fseek(file, 0, SEEK_SET);

    struct bvr_buffer_s packet;
    struct bvri_oggpage_s page = {0};
    struct bvri_vorbis_s vorbis = {0};

    packet.data = NULL;
    packet.size = 0;

    while (bvri_oggnext(file, &page, &packet))
    {
        if(BVR_STRNCMP(&packet.data[1], "vorbis", 6)){
            bvri_vorbis_decodeheader(&vorbis, packet.data, packet.elemsize);
        }
        else {
            bvri_vorbis_decodeaudio(&vorbis, packet.data, packet.elemsize);
        }
    }

    // copy data
    audio->wave = NULL;
    audio->wave_length = 0;
    audio->channels = vorbis.channels;
    audio->sample_rate = vorbis.sample_rate;

    // clear
    for (size_t i = 0; i < vorbis.codebook_count; i++)
    {
        if(vorbis.codebooks[i].entries && vorbis.codebooks[i].dimension){
            free(vorbis.codebooks[i].lenghts);
            free(vorbis.codebooks[i].vqs);

            vorbis.codebooks[i].lenghts = NULL;
            vorbis.codebooks[i].vqs = NULL;
        }
    }
    
    return BVR_TRUE;
}

#endif

static short bvri_int8_to_short(int8* v){
    return *v << 2;
}

static short bvri_int32_to_short(int8* v){
    int32 rounded = *((int32*)v) + 0xff;
    if(rounded > BVR_INT32_MAX) rounded = BVR_INT32_MAX; // avoid overflow
    return (short)(rounded >> 16);
}

static void bvri_quantize_audio(bvr_audio_t* audio, const bvr_audio_mixer_t* mixer){
    BVR_ASSERT(audio);

    if(!audio->wave){
        return;
    }

    if(audio->sample_rate != mixer->sample_rate){
        BVR_PRINT("sample rates doesn't match");
        // no op
    }

    if(audio->sample_depth != mixer->sample_depth){
        BVR_PRINT("sample depth doesn't match");

        short (*convert_short_func)(int8*) = NULL;
        switch (audio->format)
        {
        case BVR_AUDIO_FORMAT_INT8: convert_short_func = bvri_int8_to_short; break;
        case BVR_AUDIO_FORMAT_INT32: convert_short_func = bvri_int32_to_short; break;
        default:
            break;
        }

        BVR_ASSERT(convert_short_func);

        uint32 src_length = audio->wave_length;
        uint32 new_length = (uint32)((float)audio->wave_length / ((float)audio->sample_depth / (float)mixer->sample_depth));

        char* src = (char*)audio->wave;
        short* dest = calloc(new_length, sizeof(char));
        
        BVR_ASSERT(src);
        BVR_ASSERT(dest);

        for (size_t i = 0; i < src_length / audio->sample_depth; i++)
        {
            dest[i] = convert_short_func(src);
            src += audio->sample_depth;
        }
        
        free(audio->wave);

        audio->wave = dest;
        audio->wave_length = new_length;
    }
}

int bvr_create_audiof(bvr_audio_t* audio, FILE* file, const char* name){
    BVR_ASSERT(audio);
    BVR_ASSERT(file);

    // check for audio stream
    bvr_audio_mixer_t* mixer = &BVR_INSTANCE()->mixer;
    BVR_ASSERT(mixer->avail);
    
    int status = 0;

    bvr_create_string(&audio->name, name);

    audio->wave = NULL;
    audio->wave_length = 0;
    audio->channels = 0;
    audio->sample_rate = 0;
    audio->sample_depth = 0;

#ifndef BVR_NO_VORBIS
    if(bvri_is_ogg(file)){

        // check for vorbis format
        if(bvri_is_vorbis(file)){
            status |= bvri_load_vorbis(file, audio);
        }
    }
#endif

#ifndef BVR_NO_WAV
    if(bvri_is_wav(file)){
        status |= bvri_load_wav(file, audio);
    }
#endif

    // normalize audio for the targetted output
    bvri_quantize_audio(audio, mixer);

    BVR_PRINTF(
        "audio infos: \nduration: %fs\nsample rate: %i\nsample depth: %i\nchannels: %i\n", 
        audio->duration, audio->sample_rate, audio->sample_depth, audio->channels
    );

    return status;
}

#pragma endregion

#pragma region AUDIO MIXER

#ifdef _WIN32

#elif __unix__

static struct pw_stream_events __pipewire_global_events;
static struct spa_hook __pipewire_thread;

void bvri_create_mixer_impl(bvr_audio_mixer_t* mixer){
    const struct spa_pod* params[1];
    uint8 buffer[BVR_BUFFER_SIZE];

    struct spa_audio_info_raw spa_info = {0};
    struct spa_pod_builder spa_builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

    // register audio callback
    __pipewire_global_events = (struct pw_stream_events){0};
    __pipewire_global_events.version = PW_VERSION_STREAM_EVENTS;
    __pipewire_global_events.process = bvri_write_audio_stream_impl;
    
    
    // find the correct format
    switch (mixer->format)
    {
    case BVR_AUDIO_FORMAT_FLOAT: spa_info.format = SPA_AUDIO_FORMAT_F32; break;
    case BVR_AUDIO_FORMAT_UINT8: spa_info.format = SPA_AUDIO_FORMAT_U8; break;
    case BVR_AUDIO_FORMAT_INT8: spa_info.format = SPA_AUDIO_FORMAT_S8; break;
    case BVR_AUDIO_FORMAT_INT16: spa_info.format = SPA_AUDIO_FORMAT_S16; break;
    case BVR_AUDIO_FORMAT_INT32: spa_info.format = SPA_AUDIO_FORMAT_S32; break;
    default: spa_info.format = SPA_AUDIO_FORMAT_S16; break;
    }

    spa_info.channels = mixer->channels;
    spa_info.rate = mixer->sample_rate;

    pw_init(NULL, NULL);

    // create the audio callback loop
    mixer->client.pipewire.loop = pw_thread_loop_new(BVR_CLASS_NAME "-ALOOP\0", NULL);
    BVR_ASSERT(mixer->client.pipewire.loop);

    mixer->client.pipewire.context = pw_context_new(
        pw_thread_loop_get_loop(mixer->client.pipewire.loop),
        NULL, 0
    );
    BVR_ASSERT(mixer->client.pipewire.context);

    mixer->client.pipewire.core = pw_context_connect(
        mixer->client.pipewire.context, NULL, 0
    );
    BVR_ASSERT(mixer->client.pipewire.core);

    mixer->client.pipewire.stream = pw_stream_new(
        mixer->client.pipewire.core,
        BVR_CLASS_NAME "-ASTREAM", 
        pw_properties_new(
            PW_KEY_MEDIA_TYPE, "Audio",
            PW_KEY_MEDIA_CATEGORY, "Playback",
            PW_KEY_MEDIA_ROLE, "Game",
            NULL
        )
    );
    BVR_ASSERT(mixer->client.pipewire.stream);

    pw_stream_add_listener(
        mixer->client.pipewire.stream,
        &__pipewire_thread, &__pipewire_global_events,
        mixer
    );

    params[0] = spa_format_audio_raw_build(
        &spa_builder, SPA_PARAM_EnumFormat, &spa_info
    );

    pw_stream_connect(
        mixer->client.pipewire.stream,
        PW_DIRECTION_OUTPUT,
        PW_ID_ANY,
        PW_STREAM_FLAG_AUTOCONNECT | 
        PW_STREAM_FLAG_MAP_BUFFERS |
        PW_STREAM_FLAG_RT_PROCESS,
        params, 1
    );

    pw_thread_loop_start(mixer->client.pipewire.loop);
}

void bvri_write_audio_stream_impl(void* userdata){
    bvr_audio_mixer_t* mixer = (bvr_audio_mixer_t*)userdata;

    struct pw_buffer* pw_buffer;
    struct spa_buffer* spa_buffer;

    uint32 done = 0;
    short value;
    short* destination;
    const uint8 depth = mixer->sample_depth;

    pw_thread_loop_lock(mixer->client.pipewire.loop);

    if(((pw_buffer = pw_stream_dequeue_buffer(mixer->client.pipewire.stream)) == NULL)){
        BVR_PRINT("end of pipewire's buffer");
        pw_thread_loop_unlock(mixer->client.pipewire.loop);
        return;
    }

    spa_buffer = pw_buffer->buffer;
    if((destination = spa_buffer->datas[0].data) == NULL){
        // invalid spa buffer
        pw_thread_loop_unlock(mixer->client.pipewire.loop);
        return;
    }

    // find the maximum available buffer length
    mixer->master.requested_length = spa_buffer->datas[0].maxsize / (depth * mixer->channels);
    if(pw_buffer->requested){
        mixer->master.requested_length = MIN(pw_buffer->requested, mixer->master.requested_length);
    }
    
    // write buffer
    {
        memset(mixer->master.pcm, 0, mixer->master.requested_length * depth * mixer->channels);

        for (size_t i = 0; i < mixer->command_count; i++)
        {
            if(bvr_audio_do_wave_command(&mixer->commands[i]) > 1){
                mixer->commands[done++] = mixer->commands[i];
            }
        }

        mixer->command_count = done;
    }
    
    memcpy(destination, mixer->master.pcm, mixer->master.requested_length * depth * mixer->channels);

    spa_buffer->datas[0].chunk->offset = 0;
    spa_buffer->datas[0].chunk->stride = depth * mixer->channels;
    spa_buffer->datas[0].chunk->size = mixer->master.requested_length * depth * mixer->channels;

    pw_stream_queue_buffer(mixer->client.pipewire.stream, pw_buffer);
    pw_thread_loop_unlock(mixer->client.pipewire.loop);
}

void bvri_destroy_mixer_impl(bvr_audio_mixer_t* mixer){
    pw_thread_loop_stop(mixer->client.pipewire.loop);
    pw_stream_destroy(mixer->client.pipewire.stream);
    pw_thread_loop_destroy(mixer->client.pipewire.loop);

    mixer->client.pipewire.stream = NULL;
    mixer->client.pipewire.loop = NULL;
}

#endif

void bvr_audio_play(bvr_audio_t* audio, uint8 track){
    BVR_ASSERT(audio);
    BVR_ASSERT(audio->wave);

    struct bvr_audio_command_s cmd;
    cmd.id = bvr_hash(audio->name.string);
    cmd.wave = audio->wave;
    cmd.sample_depth = audio->sample_depth;
    cmd.sample_count = audio->wave_length / cmd.sample_depth;
    cmd.channels = audio->channels;
    cmd.track_id = track;

    bvr_audio_add_wave_command(&cmd);
}

void bvr_audio_stop(bvr_audio_t* audio){
    BVR_ASSERT(audio);
    BVR_ASSERT(audio->name.string);

    const uint32 hash = bvr_hash(audio->name.string);

    struct bvr_audio_command_s* cmd = bvri_audio_find_audio_command(hash);
    if(!cmd){
        return;
    }

    cmd->sample_count = 0;
    cmd->wave = NULL;
}

void bvr_destroy_audio(bvr_audio_t* audio){
    BVR_ASSERT(audio);

    bvr_destroy_string(&audio->name);

    free(audio->wave);
    audio->wave = NULL;
}

// static void bvri_audio_callback(void* _stream, int additional_amount, int total_amount){
//     uint32 done = 0;
//     bvr_audio_mixer_t* mixer = (bvr_audio_mixer_t*)_stream;
// 
//     // update buffer length in bytes
//     mixer->master.requested_length = MIN(additional_amount, BVR_AUDIO_FRAME_COUNT * sizeof(short));
// 
//     // clear previous audio buffer
//     memset(mixer->master.pcm, 0, BVR_AUDIO_FRAME_COUNT * sizeof(short));
//     
//     for (size_t i = 0; i < mixer->command_count; i++)
//     {
//         bvr_audio_do_wave_command(&mixer->commands[i]);
// 
//         if (mixer->commands[i].sample_count > 0) {
//             // if we this sample is not finished
//             mixer->commands[done++] = mixer->commands[i];
//         }
//     }
// 
//     mixer->command_count = done;
// 
//     // SDL_PutAudioStreamData(mixer->context, mixer->master.pcm, mixer->master.avail_buffer_length);
// }

int bvr_create_audio_mixer(bvr_audio_mixer_t* mixer, const int sample_rate, const uint8 channels){
    BVR_ASSERT(mixer);
    BVR_ASSERT(sample_rate > 0);

    mixer->channels = channels;
    mixer->sample_rate = sample_rate;
    mixer->sample_depth = 2;
    mixer->format = BVR_AUDIO_FORMAT;
    mixer->device_id = BVR_AUDIO_DEFAULT_OUTPUT;
    mixer->gain = 1.0f;
    mixer->command_count = 0;

    for (size_t i = 0; i < BVR_MAX_AUDIO_TRACKS; i++)
    {
        bvr_create_audio_track(mixer, i, 100.0f, 0.0f);
    }

    bvri_create_mixer_impl(mixer);

    mixer->avail = true;

    return BVR_TRUE;
}

void bvr_create_audio_track(bvr_audio_mixer_t* mixer, uint8 track, float volume, float pan){
    BVR_ASSERT(mixer);
    BVR_ASSERT(track < BVR_MAX_AUDIO_TRACKS);

    mixer->master.tracks[track].gain = volume / 100.0f;
    mixer->master.tracks[track].pan = clamp(pan, -100.0f, 100.0f);
}

/**
 * Because of how audio works, we might need to follow the same way as real sound mixers
 * https://sound-au.com/articles/audio-mixing.html
 * 
 * https://lisyarus.github.io/blog/posts/audio-mixing.html
 */
uint32 bvr_audio_do_wave_command(struct bvr_audio_command_s* command){
    bvr_audio_mixer_t* mixer = &BVR_INSTANCE()->mixer;

    // maximum available frames
    uint32 mixer_wave_frames = mixer->master.requested_length;
    uint32 cmd_wave_frames = command->sample_count / command->channels;

    uint32 frame_count = MIN(cmd_wave_frames, mixer_wave_frames);
    
    // number of samples 
    uint32 wave_samples = frame_count * command->channels;

    for (uint32 s = 0; s < frame_count; s++)
    {
        // add audio's amplitude to previous master values
        int left = (mixer->master.pcm[mixer->channels * s] + command->wave[command->channels * s]);
        int right = (mixer->master.pcm[mixer->channels * s + 1] + command->wave[command->channels * s + 1]);

        float left_volume = (mixer->master.tracks[command->track_id].pan / 100.0f + 0.5f);
        float right_volume = 1.0 - (mixer->master.tracks[command->track_id].pan / 100.0f + 0.5f);
        
        left_volume *= mixer->gain * mixer->master.tracks[command->track_id].gain;
        right_volume *= mixer->gain * mixer->master.tracks[command->track_id].gain;

        left_volume = clamp(left_volume, 0.0f, 1.0f);
        right_volume = clamp(right_volume, 0.0f, 1.0f);

        // when master if mono
        mixer->master.pcm[mixer->channels * s + 0] = (short)(clampi(left, BVR_INT16_MIN, BVR_INT16_MAX) * left_volume);

        // when master is stereo
        if(mixer->channels > 1){
            mixer->master.pcm[mixer->channels * s + 1] = (short)(clampi(left, BVR_INT16_MIN, BVR_INT16_MAX) * left_volume);
    
            // when audio clip is stereo
            if(command->channels > 1){
                mixer->master.pcm[mixer->channels * s + 1] = (short)(clampi(right, BVR_INT16_MIN, BVR_INT16_MAX) * right_volume);
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
    if(!BVR_INSTANCE()->mixer.avail){
        return;
    }

    // when there is no space for a new command
    if(BVR_INSTANCE()->mixer.command_count + 1 >= BVR_MAX_AUDIO_COMMAND){
        return;
    }

    // try to find an available command storing space
    memcpy(
        &BVR_INSTANCE()->mixer.commands[BVR_INSTANCE()->mixer.command_count++], 
        command, sizeof(struct bvr_audio_command_s)
    );
}

static struct bvr_audio_command_s* bvri_audio_find_audio_command(uint32 id){
    bvr_audio_mixer_t* mixer = &BVR_INSTANCE()->mixer;
    if(!mixer){
        return NULL;
    }

    for (size_t i = 0; i < mixer->command_count; i++)
    {
        if(mixer->commands[i].id == id){
            return &mixer->commands[i];
        }
    }
    
    return NULL;
}

void bvr_destroy_audio_mixer(bvr_audio_mixer_t* mixer){
    BVR_ASSERT(mixer);

    // SDL_CloseAudioDevice(mixer->device_id);

    bvri_destroy_mixer_impl(mixer);
    mixer->avail = false;
}

#pragma endregion