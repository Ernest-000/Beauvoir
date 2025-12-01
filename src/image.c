#include <bvr/image.h>
#include <bvr/utils.h>
#include <BVR/file.h>

#include <bvr/shader.h>
#include <bvr/scene.h>

#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <math.h>

#include <glad/glad.h>

static int bvri_get_sformat(bvr_image_t* image){
    if(image->format == 16){
        switch (image->format)
        {
        case BVR_R: return BVR_RED16;
        case BVR_RG: return BVR_RG16;
        case BVR_RGB: case BVR_BGR: return BVR_RGB16;
        case BVR_RGBA: case BVR_BGRA: return BVR_RGBA16;
        default:
            return BVR_RED16;
        }
    }

    switch (image->format)
    {
    case BVR_R: return BVR_RED8;
    case BVR_RG: return BVR_RG8;
    case BVR_RGB: case BVR_BGR: return BVR_RGB8;
    case BVR_RGBA: case BVR_BGRA: return BVR_RGBA8;
    default:
        return BVR_RED8;
    }
}

#ifndef BVR_NO_PNG

#include <png.h>

#define BVR_PNG_HEADER_LENGTH 8

static int bvri_is_png(FILE* __file) {
    fseek(__file, 0, SEEK_SET);
    uint8 header[BVR_PNG_HEADER_LENGTH];
    fread(header, 1, BVR_PNG_HEADER_LENGTH, __file);
    return png_sig_cmp(header, 0, BVR_PNG_HEADER_LENGTH) == 0;
}

static void bvri_png_error(png_structp sptr, png_const_charp cc){
    BVR_ASSERT(cc || 0);
}

static int bvri_load_png(bvr_image_t* image, FILE* file){
    png_structp pngldr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, bvri_png_error, NULL);
    BVR_ASSERT(pngldr);

    png_infop pnginfo = png_create_info_struct(pngldr);
    BVR_ASSERT(pnginfo);

    if(setjmp(png_jmpbuf(pngldr))){
        png_destroy_read_struct(&pngldr, &pnginfo, NULL);
        return BVR_FAILED;
    }

    fseek(file, BVR_PNG_HEADER_LENGTH, SEEK_SET);

    png_init_io(pngldr, file);
    png_set_sig_bytes(pngldr, BVR_PNG_HEADER_LENGTH);
    png_read_info(pngldr, pnginfo);

    image->width = png_get_image_width(pngldr, pnginfo);
    image->height = png_get_image_height(pngldr, pnginfo);
    image->depth = png_get_bit_depth(pngldr, pnginfo);
    int color_type = png_get_color_type(pngldr, pnginfo);

    if(color_type == PNG_COLOR_TYPE_PALETTE){
        png_set_palette_to_rgb(pngldr);
    }
    
    if(color_type == PNG_COLOR_TYPE_GRAY && image->depth < 8){
        png_set_expand_gray_1_2_4_to_8(pngldr);
    }

    if(png_get_valid(pngldr, pnginfo, PNG_INFO_tRNS)){
        png_set_tRNS_to_alpha(pngldr);
    }

    if(image->depth == 16){
        png_set_strip_16(pngldr);
    }
    else if(image->depth < 8){
        png_set_packing(pngldr);
    }

    png_read_update_info(pngldr, pnginfo);
    color_type = png_get_color_type(pngldr, pnginfo);

    switch (color_type)
    {
    case PNG_COLOR_TYPE_RGB:
        image->format = BVR_RGB;
        image->channels = 3;
        break;
    case PNG_COLOR_TYPE_RGBA:
        image->format = BVR_RGBA;
        image->channels = 4;
        break;
    default:
        BVR_PRINTF("color type %x is not supported!", color_type);
        png_destroy_read_struct(&pngldr, &pnginfo, NULL);
        break;
    }

    image->sformat = bvri_get_sformat(image);

    uint64 rowbytes = png_get_rowbytes(pngldr, pnginfo);
    image->pixels = malloc(image->width * image->height * image->channels * sizeof(uint8));
    BVR_ASSERT(image->pixels);

    uint8** rowp = malloc(image->height * sizeof(uint8*));
    BVR_ASSERT(rowp);

    for (uint64 i = 0; i < image->height; i++)
    {
        rowp[image->height - i - 1] = image->pixels + i * rowbytes;
    }

    png_read_image(pngldr, rowp);

    free(rowp);
    png_destroy_read_struct(&pngldr, &pnginfo, NULL);
        
    return image->pixels == NULL;
}

#endif

#ifndef BVR_NO_BMP

/*
    https://en.wikipedia.org/wiki/BMP_file_format
*/

struct bvri_bmpheader_s {
    uint8 sig[2];
    uint32 size;
    uint16 res[2];
    uint32 offset;

    uint32 header_size;
    uint32 width;
    uint32 height;
    uint16 color_plane;
    uint16 bit_per_pixel;
    uint32 compression_method;
    uint32 image_size;
    uint32 horizontal_resolution;
    uint32 vertical_resolution;
    uint32 color_palette;
    uint32 important_color;

    uint8* palette;
};

/*
    Check for signature
*/
static int bvri_is_bmp(FILE* file){
    int size;

    fseek(file, 0, SEEK_SET);    
    if(bvr_freadu8_le(file) != 'B') return 0;
    if(bvr_freadu8_le(file) != 'M') return 0;
    fseek(file, 12, SEEK_CUR);
    size = bvr_freadu32_le(file);

    return (size == 12 || size == 40 || size == 56
        || size == 108 || size == 124);
}

/*
    Return the bigger number
*/
static uint32 bvri_bmpmax(uint32 max, uint32 i){
    return i > max ? max : i;
}

static int bvri_load_bmp(bvr_image_t* image, FILE* file){
    fseek(file, 0, SEEK_SET);

    struct bvri_bmpheader_s header;

    // re-read the bitmap header
    header.sig[0] = bvr_freadu8_le(file);
    header.sig[1] = bvr_freadu8_le(file);
    header.size = bvr_freadu32_le(file);
    header.res[0] = bvr_freadu16_le(file);
    header.res[1] = bvr_freadu16_le(file);
    header.offset = bvr_freadu32_le(file);

    // DIB header
    header.header_size = bvr_freadu32_le(file);
    header.width = bvr_freadu32_le(file);
    header.height = bvr_freadu32_le(file);
    header.color_plane = bvr_freadu16_le(file);
    header.bit_per_pixel = bvr_freadu16_le(file);
    header.compression_method = bvr_freadu32_le(file);
    header.image_size = bvr_freadu32_le(file);
    header.horizontal_resolution = bvr_freadu32_le(file);
    header.vertical_resolution = bvr_freadu32_le(file);
    header.color_palette = bvr_freadu32_le(file);
    header.important_color = bvr_fread32_le(file);
    header.palette = NULL;

    // check for correct color plane
    if(header.color_plane != 1){
        BVR_PRINT("wrong color plane!");
        return BVR_FAILED;
    }
    
    // check for bitmasks
    if(header.compression_method == 3){
        fseek(file, 12, SEEK_CUR);
    }
    else if(header.compression_method == 6){
        fseek(file, 16, SEEK_CUR);
    }

    // create color palette
    if(header.bit_per_pixel < 8){
        header.palette = malloc(header.color_palette * 3);
        for (uint64 color = 0; color < header.color_palette; color++)
        {
            header.palette[color * 3 + 0] = bvr_freadu8_le(file);
            header.palette[color * 3 + 1] = bvr_freadu8_le(file);
            header.palette[color * 3 + 2] = bvr_freadu8_le(file);
            bvr_freadu8_le(file);

            BVR_PRINTF("palette color %i %i %i", header.palette[color * 3], header.palette[color * 3 + 1], header.palette[color * 3 + 2]);
        }   
    }

    // seek to pixel array
    fseek(file, header.offset, SEEK_SET);

    image->width = header.width;
    image->height = abs(header.height);
    image->depth = 8;
    
    // define correct channel and format based on bpp
    if(header.bit_per_pixel < 8){
        image->channels = 3;
        image->format = BVR_BGR;
    }
    else {
        image->channels = header.bit_per_pixel / 8;
        image->format = BVR_BGR;

        if(image->channels == 4){
            image->format = BVR_BGRA;
        }
    }

    image->sformat = bvri_get_sformat(image);
    image->pixels = malloc(image->width * image->height * image->channels);
    BVR_ASSERT(image->pixels);

    // RAW compression
    if(header.compression_method == 0){
        uint32 packed_bytes = 0;
        uint32 unpacked_bytes = 0;
        uint32 stride_length = ((int)ceilf(image->width * header.bit_per_pixel) / 32 * 4 + 3) & ~3;

        // load image from palette
        if(header.bit_per_pixel < 8){
            // because repetitive image might have a shorter stride length
            // we need to duplicate pixels to copy the correct number of pixels to get the full width.
            uint32 stride_rle = image->width / stride_length;
            uint8 buffer[4];

            for (uint64 row = 0; row < image->height; row++)
            {
                packed_bytes = 0;
                unpacked_bytes = 0;

                while (packed_bytes < stride_length)
                {
                    // copy packed data into 
                    memcpy(
                        buffer,
                        header.palette + bvri_bmpmax(header.color_palette - 1, bvr_freadu8_le(file)) * image->channels,
                        image->channels * sizeof(uint8)
                    );         
                           
                    while (packed_bytes * stride_rle - unpacked_bytes)
                    {
                        // copy the buffer 'stride_rle' times
                        memcpy(
                            image->pixels + (row * image->width + unpacked_bytes) * image->channels,
                            buffer, image->channels * sizeof(uint8)
                        );

                        unpacked_bytes++;
                    }
                    
                    packed_bytes++;
                }
            }            
        }
        // load image with raw data
        else {
            // we just copy all data row per row
            for (uint64 row = 0; row < image->height; row++)
            {
                packed_bytes = fread(image->pixels + row * image->width * image->channels, sizeof(uint8), stride_length, file);
                BVR_ASSERT(packed_bytes == stride_length);
            }
        }
    }
    else {
        BVR_ASSERT(0 || "compression not supported");
    }

    // try to free color palette
    free(header.palette);

    return BVR_OK;
}

#endif

#ifndef BVR_NO_TIF

struct bvri_tififd_s {
    short count;
    struct bvri_tiftag_s {
        short id;
        short data_type;
        int data_count;
        int data_offset;
    }* tags;
    int next;
};

struct bvri_tifframe {
    uint32 width;
    uint32 height;
    uint16 compression;

    uint32 rows_per_strip;
    uint32* strip_offsets;
    uint32* strip_byte_counts;
    uint32 strip_count;
    uint16 samples_per_pixel;
    uint32 bits_per_sample;
    uint32 bit_count;
    uint16 planar_configuration;
    uint16 sample_format;
    uint32 image_length;
    uint8 orientation;
    uint8 fill_order;
    uint32 photometric_interpretation;
    uint8 is_tiled;

    /*uint64_t photoshop_infos_count;
    uint8_t* photoshop_infos;*/
};

static int bvri_is_tif(FILE* file){
    fseek(file, 0, SEEK_SET);
    char sig1 = bvr_freadu8_le(file);
    char sig2 = bvr_freadu8_le(file);

    uint16 version = bvr_fread16_le(file);
    uint32 offset = bvr_fread32_le(file);

    return (sig1 == 'I' || sig1 == 'M') 
        && (sig2 == 'I' || sig2 == 'M') 
        && version == 42;
}

/*
    Copy TIF data from a buffer into a pointer.
*/
static void bvri_tif_copy_data(FILE* f, int offset, int size, void* data){
    uint64 prev = ftell(f);
    fseek(f, offset, SEEK_SET);
    fread(data, sizeof(char), size, f);
    fseek(f, prev, SEEK_SET);
}

/*
// https://stackoverflow.com/questions/36035074/how-can-i-find-an-overlap-between-two-given-ran
static int bvri_tif_do_ranges_overlap(uint64_t xstart, uint64_t xend, uint64_t ystart, uint64_t yend,
    uint64_t* overlap_start, uint64_t* overlap_end){
    
    size_t range = fmax(xend, yend) - fmin(xstart, ystart);
    size_t sum = (xend - xstart) + (yend - ystart);

    if(sum > range){
        if(overlap_end) {
            *overlap_end = fmin(xend, yend);
        }
        if(overlap_start){
            *overlap_start = fmax(xstart, ystart);
        }
        return BVR_OK;
    }

    return BVR_FAILED;
}
*/

/*
    Return the size of each tag's types
*/
static uint32 bvri_tif_sizeof(uint32 size){
    switch (size)
    {
    case 1: case 2:
        return sizeof(char);
    case 3: return sizeof(short);
    case 4: return sizeof(int);
    
    default:
        return 0;
    }
}

/*
    Sources :
    https://github.com/jkriege2/TinyTIFF/blob/master/src/tinytiffreader.c
    https://www.fileformat.info/format/tiff/egff.htm
*/
static int bvri_load_tif(bvr_image_t* image, FILE* file){
    fseek(file, 0, SEEK_SET);
    bvr_fread32_le(file); // id & version
    int idf_offset = bvr_fread32_le(file);

    struct bvri_tififd_s idf;
    struct bvri_tifframe frame;
    idf.count = 0;
    idf.tags = NULL;
    idf.next = idf_offset;

    // while we got a next image header
    while (idf.next)
    {
        // clear frame's data.
        memset(&frame, 0, sizeof(struct bvri_tifframe));
        
        // seek to the first bit
        fseek(file, idf.next, SEEK_SET);
        uint16 tag_count = bvr_fread16_le(file); // number of tags
        idf.tags = malloc(sizeof(struct bvri_tiftag_s) * tag_count);
        BVR_ASSERT(idf.tags);
        
        // read tags data from file.
        fread(idf.tags, sizeof(struct bvri_tiftag_s), tag_count, file);

        // find each tags
        for (uint64 tagi = 0; tagi < tag_count; tagi++)
        {
            int integer_id;
            memcpy(&integer_id, &idf.tags[tagi].id, sizeof(short));

            switch (integer_id)
            {
            case 257:{ // height
                    frame.height = idf.tags[tagi].data_offset;
                    frame.image_length = frame.height;
                }
                break;
            case 256:{ // width
                    frame.width = idf.tags[tagi].data_offset;
                }
                break;
            case 258: { // bit per sample
                    frame.bit_count = idf.tags[tagi].data_count;
                    // we get each component sizes and add them together
                    for (uint64 ii = 0; ii < idf.tags[tagi].data_count; ii++)
                    {
                        short bpp;
                        bvri_tif_copy_data(file, idf.tags[tagi].data_offset, sizeof(short), &bpp);
                        frame.bits_per_sample += bpp;
                    }
                }
                break;
            case 259:{ // compression
                    frame.compression = idf.tags[tagi].data_offset;
                }
                break;
            case 262: { // PhotometricInterpretation
                    frame.photometric_interpretation = idf.tags[tagi].data_offset;
                }
                break;
            case 273: { // strip offsets
                    if(!frame.strip_offsets){
                        frame.strip_count = idf.tags[tagi].data_count;
                        frame.strip_offsets = calloc(frame.strip_count, sizeof(uint32));
                        if(frame.strip_offsets){
                            bvri_tif_copy_data(file, idf.tags[tagi].data_offset, 
                                bvri_tif_sizeof(idf.tags[tagi].data_type) * idf.tags[tagi].data_count, 
                                frame.strip_offsets
                            );
                        }
                        else {BVR_ASSERT(0 || "failed to allocate strip offset!");}
                    }
                }
                break;
            case 277: { // sample per pixel
                    frame.samples_per_pixel = idf.tags[tagi].data_offset;
                }
                break;
            case 278: { // row per strip
                    frame.rows_per_strip = idf.tags[tagi].data_offset;
                }
                break;
            case 339: { // sample format
                    frame.sample_format = idf.tags[tagi].data_offset;
                }
                break;
            case 279: {
                    if(!frame.strip_byte_counts){
                        frame.strip_count = idf.tags[tagi].data_count;
                        frame.strip_byte_counts = calloc(frame.strip_count, sizeof(uint32));
                        if(frame.strip_byte_counts){
                            bvri_tif_copy_data(file, idf.tags[tagi].data_offset, 
                                bvri_tif_sizeof(idf.tags[tagi].data_type) * idf.tags[tagi].data_count, 
                                frame.strip_byte_counts
                            );
                        }
                        else {BVR_ASSERT(0 || "failed to allocate strip byte offset!");}
                    }
                }
                break;
            case 284: { // planar config
                    frame.planar_configuration = idf.tags[tagi].data_offset;
                }
                break;
            case 274: { // image orientation
                    frame.orientation = idf.tags[tagi].data_offset;
                }
                break;
            case 266: { // fill order
                    frame.fill_order = idf.tags[tagi].data_offset;
                }
                break;
            case 325:
            case 323:
            case 324:
            case 322: { // all of those tags means that it's tiled-based
                    frame.is_tiled = 1;
                }
                break;
            /*
            case 37724: { // TODO: handle photoshop's tags
                    if(!frame.photoshop_infos){
                        frame.photoshop_infos_count = idf.tags[tagi].data_count;
                        frame.photoshop_infos = calloc(idf.tags[tagi].data_count, sizeof(uint8_t));
                        BVR_PRINTF("photoshop offset %i", idf.tags[tagi].data_offset);
                        
                        if(frame.photoshop_infos){
                            bvri_tif_copy_data(file, idf.tags[tagi].data_offset,
                                bvri_tif_sizeof(idf.tags[tagi].data_type) * idf.tags[tagi].data_count,
                                frame.photoshop_infos 
                            );
                        }
                        else {BVR_ASSERT(0 || "failed to allocate photoshop informations!");}
                    }
                }
            */
            default:
                break;
            }
        }

        BVR_ASSERT(frame.compression == 1); // other compressions are not supported
        BVR_ASSERT(frame.is_tiled == 0); // tilling is not supported
        BVR_ASSERT(frame.orientation == 1); // other orientations are not supported
        BVR_ASSERT(frame.photometric_interpretation != 3); // palettes are not supported
        BVR_ASSERT(frame.width > 0 && frame.height > 0);
        BVR_ASSERT(frame.bits_per_sample == 8 
                    || frame.bits_per_sample == 16
                    || frame.bits_per_sample == 24
                    || frame.bits_per_sample == 32);
        
        image->width = frame.width;
        image->height = frame.height;
        image->layers.size += sizeof(bvr_layer_t);
        image->depth = frame.samples_per_pixel;

        if(frame.planar_configuration == 1){
            /*BVR_PRINTF("strip count %i", frame.strip_count);
            for (size_t strip = 0; strip < frame.strip_count; strip++)
            {
                BVR_PRINTF("start %i end %i", frame.strip_offsets[strip], frame.strip_byte_counts[strip]);    
            }*/

            BVR_ASSERT(0 || "configuration not supported!");
        }
        else if(frame.planar_configuration == 2) {
            /*
                Each color layers are stored in a different strip
                strip 1 -> R 
                strip 2 -> G 
                strip 3 -> B 
                strip 4 -> A
            */
            if(image->pixels){
                // free previous allocated memory
                free(image->pixels);
            }

            image->channels = frame.strip_count;
            image->pixels = malloc(image->width * image->height * image->channels);
            BVR_ASSERT(image->pixels);

            for (uint64 strip = 0; strip < frame.strip_count; strip++)
            {
                uint8* strip_buffer = calloc(frame.strip_byte_counts[strip], sizeof(uint8));
                BVR_ASSERT(strip_buffer);

                // read the entire strip into a buffer
                fseek(file, frame.strip_offsets[strip], SEEK_SET);
                fread(strip_buffer, sizeof(uint8), frame.strip_byte_counts[strip], file);

                uint64 image_index = strip;
                for (uint64 strip_index = 0; strip_index < frame.strip_byte_counts[strip]; strip_index++)
                {
                    // copy each pixels into the final image buffer
                    image->pixels[image_index] = strip_buffer[strip_index];
                    image_index += image->channels;
                }
                
                free(strip_buffer);
            }

        }
        else {
            BVR_ASSERT(0 || "configuration is not supported!");
        }

        switch (image->channels)
        {
        case 1: image->format = BVR_R; break;
        case 2: image->format = BVR_RG; break;
        case 3: image->format = BVR_RGB; break;
        case 4: image->format = BVR_RGBA; break;
        
        default:
            break;
        }

        image->sformat = bvri_get_sformat(image);

        // free ressources
        free(idf.tags);
        free(frame.strip_offsets);
        free(frame.strip_byte_counts);
        //free(frame.photoshop_infos);

        // seek at the end of the image descriptor header
        fseek(file, idf.next + 2 + sizeof(struct bvri_tiftag_s) * tag_count, SEEK_SET);

        // define next image descriptor header
        idf.next = bvr_fread32_le(file);
        if(idf.next){
            BVR_PRINT("using multiple framed TIF files might overwrite previous data!");
        }
    }

    return BVR_OK;
}

#endif

#ifndef BVR_NO_PSD

struct bvri_psdheader_s {
    char sig[4];
    short version;
    char res[6];
    short channels;

    uint32 rows;
    uint32 columns;
    short depth;

    // Bitmap = 0; Grayscale = 1; Indexed = 2; RGB = 3; CMYK = 4;
    // Multichannel = 7; Duotone = 8; Lab = 9.
    short mode; 
};

struct bvri_psdlayer_s {
    // top, left, bottom, right
    uint32 bounds[4];
    short channel_count;
    struct bvri_psdlayerchannel_s {
        short id;
        uint32 length;
        uint64 position;
    }* channels;
    
    char sig[5];
    int blend_mode;
    uint8 opacity;
    char clipping;
    char flags;

    bvr_string_t name;
};

struct bvri_psdressource_s {
    char sig[5];                // ressource block signature
    short id;                   // ressource id
    bvr_string_t name;          // name
    struct bvr_buffer_s data;   // pointer to the data
};

static int bvri_is_psd(FILE* file){
    uint8 sig[5];
    short version;

    fseek(file, 0, SEEK_SET);
    fread(sig, sizeof(char), 4, file);
    sig[4] = '\0';

    version = bvr_freadu16_be(file);
    return strcmp(sig, "8BPS") == 0 //8BPS -> psd's magic number 
            && version == 1; 
}

/*
    Create a new string from PSD's pascal-typed string
*/
static void bvri_psd_read_pascal_string(bvr_string_t* string, FILE* file){
    string->string = NULL;
    string->length = (uint64)bvr_freadu8_le(file) + 1;

    if(string->length - 1){
        string->string = malloc(string->length);
        BVR_ASSERT(string->string);

        fread(string->string, sizeof(char), string->length - 1, file);
        string->string[string->length - 1] = '\0';
    }
}

/*
    Sources :
    https://docs.fileformat.com/image/psd/
    https://www.fileformat.info/format/psd/egff.htm
    https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_pgfId-1030196
    https://en.wikipedia.org/wiki/PackBits
*/
static int bvri_load_psd(bvr_image_t* image, FILE* file){
    struct bvri_psdheader_s header;
    
    struct {
        uint32 size;
        uint8* data;
    } color_mode_section;

    struct {
        uint32 size; // ressource's data full size
        uint32 end_position;
        uint32 count; //
        struct bvri_psdressource_s block;
    } ressources_section;

    struct {
        uint64 size;
        uint64 end_position;

        uint64 layer_size;
        uint64 mask_size;

        uint8 next_alpha_channel_is_global;
        short layer_count;
        struct bvri_psdlayer_s* layers;
    } layer_section;

    struct {
        short compression;
        short channels;
        short channel;
        uint32 rows;
        uint32 columns;
        uint32 unpacked_length;
        uint32 packed_length;
        uint8* unpacked_buffer;
        uint8* packed_buffer;
        uint16* rle_pack_lengths;
    } image_data_section;

    // reading psd's header
    // skip sig header
    fseek(file, 4, SEEK_SET);
    header.version = bvr_freadu16_be(file);
    fseek(file, 6, SEEK_CUR); // skip reserved
    header.channels = bvr_freadu16_be(file);
    header.rows = bvr_freadu32_be(file);
    header.columns = bvr_freadu32_be(file);
    header.depth = bvr_freadu16_be(file);
    header.mode = bvr_freadu16_be(file);

    // check for color mode section (if the size == 0, no section)
    color_mode_section.size = bvr_freadu32_be(file);
    color_mode_section.data = NULL;
    if(color_mode_section.size){
        BVR_PRINTF("color mode %i, should read full data", color_mode_section.size);
        BVR_ASSERT(0 || "not supported!");
    }

    // ressource section parsing
    ressources_section.size = bvr_freadu32_be(file);
    ressources_section.end_position = ftell(file) + ressources_section.size;
    
    if(ressources_section.size){
        while (ftell(file) < ressources_section.end_position)
        {
            bvr_freadstr(ressources_section.block.sig, sizeof(ressources_section.block.sig), file);
            
            // check for signature
            BVR_ASSERT(strncmp(ressources_section.block.sig, "8BIM", 4) == 0);

            ressources_section.block.id = bvr_freadu16_be(file);

            bvri_psd_read_pascal_string(&ressources_section.block.name, file);
            bvr_freadu8_le(file); // filler byte

            ressources_section.block.data.size = bvr_freadu32_be(file);
            ressources_section.block.data.elemsize = ressources_section.block.data.size;
            ressources_section.block.data.data = NULL;

            // seek to the end of the section. Each section's size must be even. 
            fseek(file, (ressources_section.block.data.size + 1) & ~1, SEEK_CUR);
            bvr_destroy_string(&ressources_section.block.name);
            
            free(ressources_section.block.data.data);
            ressources_section.block.data.data = NULL;
        }
        
        ressources_section.count++;

        fseek(file, ressources_section.end_position, SEEK_SET);
    }

    layer_section.size = bvr_freadu32_be(file);
    layer_section.end_position = ftell(file) + layer_section.size;
    {
        uint64 start_of_the_header = ftell(file);

        layer_section.next_alpha_channel_is_global = 0;
        layer_section.layer_size = bvr_freadu32_be(file);
        layer_section.layer_count = bvr_freadu16_be(file);

        if(layer_section.layer_count < 0){
            layer_section.layer_count = -layer_section.layer_count;
        }
        else {
            layer_section.next_alpha_channel_is_global = 1;
        }

        layer_section.layers = NULL;
        layer_section.layers = calloc(layer_section.layer_count, sizeof(struct bvri_psdlayer_s));
        BVR_ASSERT(layer_section.layers);
        
        struct bvri_psdlayer_s* layer;
        for (uint64 layer_id = 0; layer_id < layer_section.layer_count; layer_id++)
        {
            uint64 end_of_header;

            layer = &layer_section.layers[layer_id];

            layer->bounds[0] = bvr_freadu32_be(file);
            layer->bounds[1] = bvr_freadu32_be(file);
            layer->bounds[2] = bvr_freadu32_be(file);
            layer->bounds[3] = bvr_freadu32_be(file);

            layer->channel_count = bvr_freadu16_be(file);

            // skip channel info???
            layer->channels = calloc(layer->channel_count, sizeof(struct bvri_psdlayerchannel_s));
            for (uint64 channel = 0; channel < layer->channel_count; channel++)
            {
                layer->channels[channel].id = bvr_freadu16_be(file);
                layer->channels[channel].position = 0;
                layer->channels[channel].length = bvr_freadu32_be(file);
            }
            
            bvr_freadstr(layer->sig, 5, file);

            // get blend mode
            layer->blend_mode = bvr_freadu32_be(file);
            switch (layer->blend_mode)
            {
                case 0x70617373: layer->blend_mode = BVR_LAYER_BLEND_PASSTHROUGH; break;
                case 0x6E6F726D: layer->blend_mode = BVR_LAYER_BLEND_NORMAL; break;
                case 0x64697373: layer->blend_mode = BVR_LAYER_BLEND_DISSOLVE; break;
                case 0x6461726B: layer->blend_mode = BVR_LAYER_BLEND_DARKEN; break;
                case 0x6D756C00: layer->blend_mode = BVR_LAYER_BLEND_MULTIPLY; break;
                case 0x69646976: layer->blend_mode = BVR_LAYER_BLEND_COLORBURN; break;
                case 0x6C62726E: layer->blend_mode = BVR_LAYER_BLEND_LINEARBURN; break;
                case 0x646B436C: layer->blend_mode = BVR_LAYER_BLEND_DARKERCOLOR; break;
                case 0x6C697465: layer->blend_mode = BVR_LAYER_BLEND_LIGHTEN; break;
                case 0x7363726E: layer->blend_mode = BVR_LAYER_BLEND_SCREEN; break;
                case 0x64697600: layer->blend_mode = BVR_LAYER_BLEND_COLORDODGE; break;
                case 0x6C646467: layer->blend_mode = BVR_LAYER_BLEND_LINEARDODGE; break;
                case 0x6C67436C: layer->blend_mode = BVR_LAYER_BLEND_LIGHTERCOLOR; break;
                case 0x6F766572: layer->blend_mode = BVR_LAYER_BLEND_OVERLAY; break;
                case 0x734C6974: layer->blend_mode = BVR_LAYER_BLEND_SOFTLIGHT; break;
                case 0x684C6974: layer->blend_mode = BVR_LAYER_BLEND_HARDLIGHT; break;
                case 0x764C6974: layer->blend_mode = BVR_LAYER_BLEND_VIVIDLIGHT; break;
                case 0x6C4C6974: layer->blend_mode = BVR_LAYER_BLEND_LINEARLIGHT; break;
                case 0x704C6974: layer->blend_mode = BVR_LAYER_BLEND_PINLIGHT; break;
                case 0x684D6978: layer->blend_mode = BVR_LAYER_BLEND_HARDMIX; break;
                case 0x64696666: layer->blend_mode = BVR_LAYER_BLEND_DIFFERENCE; break;
                case 0x736D7564: layer->blend_mode = BVR_LAYER_BLEND_EXCLUSION; break;
                case 0x66737566: layer->blend_mode = BVR_LAYER_BLEND_SUBSTRACT; break;
                case 0x66646976: layer->blend_mode = BVR_LAYER_BLEND_DIVIDE; break;
                case 0x68756500: layer->blend_mode = BVR_LAYER_BLEND_HUE; break;
                case 0x73617400: layer->blend_mode = BVR_LAYER_BLEND_SATURATION; break;
                case 0x636F6C72: layer->blend_mode = BVR_LAYER_BLEND_COLOR; break;
                case 0x65756D00: layer->blend_mode = BVR_LAYER_BLEND_LUMINOSITY; break;
                default:
                break;
            }

            BVR_ASSERT(strncmp(layer->sig, "8BIM", 4) == 0);
            // TODO: define blend mode

            layer->opacity = bvr_freadu8_be(file);
            layer->clipping = bvr_freadu8_be(file);
            layer->flags = bvr_freadu8_be(file);
            bvr_freadu8_be(file); // filler bit

            end_of_header = ftell(file) + bvr_freadu32_be(file) + 4; 

            fseek(file, bvr_freadu32_be(file), SEEK_CUR); // skip Layer mask / adjustment layer data
            fseek(file, bvr_freadu32_be(file), SEEK_CUR); // skip Layer blending ranges data
            
            bvri_psd_read_pascal_string(&layer->name, file);

            // pascal string padding.
            fseek(file, (layer->name.length - 1) - (((layer->name.length - 1) / 4) * 4) + 3, SEEK_CUR);

            // global layer mask info
            fseek(file, bvr_freadu32_be(file), SEEK_CUR);

            int has_next_additional_data = 1;
            while(has_next_additional_data) {
                char additional_data_sig[5];
                char additional_data_tag[5];

                bvr_freadstr(additional_data_sig, sizeof(additional_data_sig), file);

                if(strncmp(additional_data_sig, "8BIM", 4) == 0 || strncmp(additional_data_sig, "8B64", 4) == 0){
                    uint64 data_size;
                    
                    bvr_freadstr(additional_data_tag, sizeof(additional_data_tag), file);
                    // TODO: check tags

                    data_size = (bvr_freadu32_be(file) + 1) & ~1;
                    fseek(file, data_size, SEEK_CUR);
                }
                else {
                    has_next_additional_data = 0;
                }
            }

            fseek(file, end_of_header, SEEK_SET);
        }
    }

    //image->channels = header.channels;
    image->channels = 4; // we force 4 channels
    image->width = header.columns;
    image->height = header.rows;
    image->depth = header.depth;

    switch (image->channels)
    {
    case 0: image->format = BVR_R; break; // monochrome
    case 1: image->format = BVR_R; break; // gray-scale
    case 2: image->format = BVR_RG; break;
    case 3: image->format = BVR_RGB; break; // rgb
    case 4: image->format = BVR_RGBA; break;
    default:
        BVR_ASSERT(0 || "image format not supported!");
        break;
    }

    image->sformat = bvri_get_sformat(image);

    image->pixels = malloc(image->width * image->height * image->channels * layer_section.layer_count);

    image->layers.size = layer_section.layer_count * image->layers.elemsize;
    image->layers.data = calloc(layer_section.layer_count, image->layers.elemsize);
    BVR_ASSERT(image->layers.data);

    // initialize layers to make sure they're correct
    for (uint64 layer = 0; layer < layer_section.layer_count; layer++)
    {
        bvr_layer_t* layer_ptr = &((bvr_layer_t*)image->layers.data)[layer];

        bvr_string_create_and_copy(&layer_ptr->name, &layer_section.layers[layer].name);
        layer_ptr->flags = 0;
        layer_ptr->blend_mode = (bvr_layer_blend_mode_t)layer_section.layers[layer].blend_mode;
        layer_ptr->width = layer_section.layers[layer].bounds[3] - layer_section.layers[layer].bounds[1];
        layer_ptr->height = layer_section.layers[layer].bounds[2] - layer_section.layers[layer].bounds[0];
        layer_ptr->anchor_x = layer_section.layers[layer].bounds[1];
        layer_ptr->anchor_y = layer_section.layers[layer].bounds[0];
        layer_ptr->opacity = layer_section.layers[layer].opacity;

        if(layer_section.layers[layer].clipping){
            layer_ptr->flags |= BVR_LAYER_CLIPPED;
        }
    }

    image_data_section.unpacked_buffer = NULL;
    image_data_section.packed_buffer = NULL;
    image_data_section.rle_pack_lengths = NULL;
    image_data_section.compression = bvr_freadu16_be(file);
    {
        if(image_data_section.compression == 0){
            // proceed to RAW uncompression 

            BVR_ASSERT(0 || "unsupported compression mode");
        }
        else if(image_data_section.compression == 1){
            // do RLE uncompression
            for (uint64 layer = 0; layer < layer_section.layer_count; layer++)
            {
                int layer_width = layer_section.layers[layer].bounds[3] - layer_section.layers[layer].bounds[1];
                int layer_height = layer_section.layers[layer].bounds[2] - layer_section.layers[layer].bounds[0];
                int layer_anchor_x = __min(layer_section.layers[layer].bounds[1], image->width - layer_width);
                int layer_anchor_y = __min(layer_section.layers[layer].bounds[0], image->height - layer_height);
            
                image_data_section.channels = layer_section.layers[layer].channel_count;
                image_data_section.rle_pack_lengths = calloc(layer_height, sizeof(uint16));
                BVR_ASSERT(image_data_section.rle_pack_lengths);

                for (uint64 channel = 0; channel < image_data_section.channels; channel++)
                {
                    uint64 readed_size;

                    layer_section.layers[layer].channels[channel].position = ftell(file);
                    image_data_section.channel = layer_section.layers[layer].channels[channel].id; 
                    switch (image_data_section.channel)
                    {
                    case -1: // transparency
                        {
                            image_data_section.columns = layer_width;
                            image_data_section.rows = layer_height;  
                            image_data_section.channel = 3;
                        }
                        break;
                    case -2: // layer or vector
                        {
                            image_data_section.columns = layer_width;
                            image_data_section.rows = layer_height;
                            image_data_section.channel = 0;
                        }
                        break;
                    case -3: // layer mask
                        {
                            image_data_section.columns = layer_width;
                            image_data_section.rows = layer_height;
                            image_data_section.channel = 0;
                        }
                        break;
                    
                    case 1: // red
                    case 2: // blue
                    case 3: // green
                    default: // color channel
                        {
                            image_data_section.columns = layer_width;
                            image_data_section.rows = layer_height;
                        }
                        break;
                    }

                    image_data_section.packed_length = 0;
                    image_data_section.unpacked_length = image_data_section.columns * image_data_section.rows;

                    for (uint64 j = 0; j < image_data_section.rows; j++)
                    {
                        image_data_section.rle_pack_lengths[j] = bvr_freadu16_be(file);
                        image_data_section.packed_length += image_data_section.rle_pack_lengths[j];
                    }
                    
                    image_data_section.packed_buffer = calloc(image_data_section.packed_length, sizeof(uint8));
                    image_data_section.unpacked_buffer = calloc(image_data_section.unpacked_length, sizeof(uint8));
                    BVR_ASSERT(image_data_section.packed_buffer);
                    BVR_ASSERT(image_data_section.unpacked_buffer);

                    readed_size = fread(image_data_section.packed_buffer, sizeof(uint8), image_data_section.packed_length, file);
                    if(readed_size != image_data_section.packed_length){
                        BVR_PRINT("skipping layer");
                        continue;
                    }

                    uint8 character = 0;
                    uint8 count = 0;
                    uint32 count_as_int = 0;
                    uint64 offset = 0;
                    uint64 readed_bytes = 0;
                    while (readed_bytes < image_data_section.packed_length 
                            && offset < image_data_section.unpacked_length)
                    {
                        BVR_ASSERT(readed_bytes < image_data_section.packed_length);
                        BVR_ASSERT(offset < image_data_section.unpacked_length);

                        if(readed_bytes < image_data_section.packed_length){
                            count = image_data_section.packed_buffer[readed_bytes++];
                            
                            if(count == 0x80){
                                // byte == 128
                                // no-op
                            }
                            else if(count > 0x80){
                                // 0x81 < byte < 0xFF
                                count_as_int = (uint32)(0x101 - count);

                                BVR_ASSERT(offset + count_as_int <= image_data_section.unpacked_length);

                                memset(&image_data_section.unpacked_buffer[offset], 
                                    image_data_section.packed_buffer[readed_bytes++],
                                    count_as_int
                                );

                                offset += count_as_int;
                            }
                            else {
                                // 0x00 < byte < 0x7F
                                count_as_int = (uint32)(count + 1);

                                BVR_ASSERT(offset + count_as_int <= image_data_section.unpacked_length);

                                memcpy(&image_data_section.unpacked_buffer[offset],
                                    &image_data_section.packed_buffer[readed_bytes],
                                    count_as_int
                                );

                                offset += count_as_int;
                                readed_bytes += count_as_int;
                            }
                        }
                        else {
                            memset(image_data_section.unpacked_buffer, 0, image_data_section.unpacked_length - offset);
                            offset = image_data_section.unpacked_length;
                        }
                    }

                    for (uint64 strip = 0; strip < image_data_section.rows; strip++)
                    {
                        for (uint64 column = 0; column < image_data_section.columns; column++)
                        {
#ifndef BVR_NO_FLIP

                            image->pixels[
                                (((strip + layer_anchor_y) * image->width + column + layer_anchor_x) * image->channels + image_data_section.channel) +
                                (image->width * image->height * image->channels * layer)
                            ] = image_data_section.unpacked_buffer[strip * image_data_section.columns + column];
#else
                            image->pixels[
                                ((strip * image->width + column) * image->channels + image_data_section.target_channel) +
                                (image->width * image->height * image->channels * layer)
                            ] = image_data_section.unpacked_buffer[strip * image_data_section.columns + column];
#endif
                        }

                    }

                    // seek to the end of the channel data
                    fseek(file, 
                        layer_section.layers[layer].channels[channel].position 
                        + layer_section.layers[layer].channels[channel].length, 
                        SEEK_SET
                    );
                    
                    free(image_data_section.packed_buffer);
                    free(image_data_section.unpacked_buffer);
                    image_data_section.packed_buffer = NULL;
                    image_data_section.unpacked_buffer = NULL;
                }
                
                free(image_data_section.rle_pack_lengths);
                image_data_section.rle_pack_lengths = NULL;
            }
            
        }
        else {
            BVR_ASSERT(0 || "unsupported compression mode");
        }
    }

    // freeing data
    for (uint64 i = 0; i < layer_section.layer_count; i++)
    {
        bvr_destroy_string(&layer_section.layers[i].name);
        
        free(layer_section.layers[i].channels);
        layer_section.layers[i].channels = NULL;
    }

    free(layer_section.layers);

    return BVR_OK;
}

#endif

/*
    Create a default layer on an image.
    This layer will have the same size as the image.
    This layer will be named "layer0".
*/
static void bvri_create_empty_layer(bvr_image_t* image){
    BVR_ASSERT(image);

    image->layers.data = malloc(image->layers.elemsize);
    image->layers.size = image->layers.elemsize;

    ((bvr_layer_t*)image->layers.data)[0].blend_mode = BVR_LAYER_BLEND_NORMAL;
    ((bvr_layer_t*)image->layers.data)[0].flags = 0;
    ((bvr_layer_t*)image->layers.data)[0].width = image->width;
    ((bvr_layer_t*)image->layers.data)[0].height = image->height;
    ((bvr_layer_t*)image->layers.data)[0].anchor_x = 0;
    ((bvr_layer_t*)image->layers.data)[0].anchor_y = 0;
    
    bvr_create_string(&((bvr_layer_t*)image->layers.data)[0].name, "layer0");
}

/*
    Flip vertically an array of pixels.
*/
static void bvri_flip_image_vertically_raw(uint8* pixels, int stride, int width, int height, int channels) {
    uint8 buffer[BVR_BUFFER_SIZE];

    for (int row = 0; row < (height >> 1); row++)
    {
        uint8* row0 = pixels + row * stride;
        uint8* row1 = pixels + (height - row - 1) * stride;
        uint64 bleft = stride;

        while (bleft)
        {
            uint64 bcpy = sizeof(buffer);
            if(bleft < sizeof(buffer)){
                bcpy = bleft;
            }

            memcpy(buffer, row0, bcpy);
            memcpy(row0, row1, bcpy);
            memcpy(row1, buffer, bcpy);

            row0 += bcpy;
            row1 += bcpy;

            bleft -= bcpy;
        }
    }
}

int bvr_create_imagef(bvr_image_t* image, FILE* file){
    BVR_ASSERT(image);
    BVR_ASSERT(file);

    int status = 0;

    image->width = 0;
    image->height = 0;
    image->depth = 0;
    image->format = 0;
    image->sformat = 0;
    image->channels = 0;
    image->pixels = NULL;
    image->layers.data = NULL;
    image->layers.size = 0;
    image->layers.elemsize = sizeof(bvr_layer_t);

    // I should change image format order so that it will reduce signature errors.
#ifndef BVR_NO_PNG
    if(bvri_is_png(file)){ 
        status = bvri_load_png(image, file);
    }
#endif

#ifndef BVR_NO_BMP
    if(bvri_is_bmp(file) && !status){
        status = bvri_load_bmp(image, file);
    }
#endif

#ifndef BVR_NO_TIF
    if(bvri_is_tif(file) && !status){
        status = bvri_load_tif(image, file);
    }
#endif

#ifndef BVR_NO_PSD
    if(bvri_is_psd(file) && !status){
        status = bvri_load_psd(image, file);
    }
#endif

#ifndef BVR_NO_FLIP
    if(image->pixels && status){
        bvr_flip_image_vertically(image);
    }
#endif

    return status;
}

int bvr_create_bitmap(bvr_image_t* bitmap, const char* path, int channel){
    BVR_ASSERT(bitmap);
    BVR_ASSERT(path);

    bvr_image_t image;
    FILE* file = fopen(path, "rb");
    bvr_create_imagef(&image, file);
    if(!image.pixels){
        fclose(file);

        BVR_PRINT("failed to open image!");
        return BVR_FAILED;
    }

    bitmap->width = image.width;
    bitmap->height = image.height;
    bitmap->depth = image.depth;
    bitmap->format = BVR_R;
    bitmap->channels = 1;
    bitmap->layers.data = NULL;
    bitmap->layers.size = 0;
    bitmap->layers.elemsize = sizeof(bvr_layer_t);

    bitmap->pixels = malloc(bitmap->width * bitmap->height);
    BVR_ASSERT(bitmap->pixels);

    memset(bitmap->pixels, 0, bitmap->width * bitmap->height);

    bvr_image_copy_channel(&image, channel, bitmap->pixels);

    fclose(file);
    bvr_destroy_image(&image);
    
    return BVR_OK;
}

void bvr_flip_image_vertically(bvr_image_t* image){
    BVR_ASSERT(image);

    for (uint64 layer = 0; layer < BVR_BUFFER_COUNT(image->layers); layer++){
        bvri_flip_image_vertically_raw(
            &image->pixels[image->width * image->height * image->channels * layer],
            image->width * image->channels, image->width, image->height, image->channels
        );
    }
}

int bvr_image_copy_channel(bvr_image_t* image, int channel, uint8* buffer){
    BVR_ASSERT(image);
    BVR_ASSERT(image->pixels);
    BVR_ASSERT(buffer);

    /*
        RED=0x0
        GREEN=0x1
        BLUE=0x2
        ALPHA=0x3
    */
    if(image->format == BVR_BGR || image->format == BVR_BGRA){
        switch (channel)
        {
        case 0x0:
            channel = 0x2;
            break;
        case 0x2:
            channel = 0x0; 
            break;
        default:
            break;
        }
    }

    for (uint64 y = 0; y < image->height; y++)
    {
        for (uint64 x = 0; x < image->width; x++)
        {
            buffer[y * image->width + x] = image->pixels[(y * image->width + x) * image->channels + channel];
        }
    }
}

void bvr_destroy_image(bvr_image_t* image){
    BVR_ASSERT(image);

    for (uint64 layer = 0; layer < BVR_BUFFER_COUNT(image->layers); layer++)
    {
        bvr_destroy_string(&((bvr_layer_t*)image->layers.data)[layer].name);
    }
    

    free(image->pixels);
    free(image->layers.data);
    image->pixels = NULL;
    image->layers.data = NULL;
}

static int bvri_create_texture_base(bvr_texture_t* texture){
    glGenTextures(1, &texture->id);
    glBindTexture(texture->target, texture->id);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->image.width);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, texture->image.height);

    glTexParameteri(texture->target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(texture->target, GL_TEXTURE_MAX_LEVEL, 1);
    glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, (int)texture->wrap);
    glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, (int)texture->wrap);
    glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, (int)texture->filter);
    glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, (int)texture->filter);

    return texture->id > 0;
}

int bvr_create_view_texture(bvr_texture_t* origin, bvr_texture_t* dest, int width, int height, int layer){
    BVR_ASSERT(origin);
    BVR_ASSERT(width > 0 && height > 0);

    dest->filter = origin->filter;
    dest->wrap = origin->wrap;
    dest->id = 0;
    dest->unit = 0;
    dest->target = GL_TEXTURE_2D;
    dest->image.width = dest->image.width;
    dest->image.height = dest->image.height;
    dest->image.depth = origin->image.depth;
    dest->image.format = origin->image.format;
    dest->image.sformat = origin->image.sformat;
    dest->image.channels = origin->image.channels;
    dest->image.pixels = NULL;
    dest->image.layers.data = NULL;
    dest->image.layers.size = 0;
    dest->image.layers.elemsize = sizeof(bvr_layer_t);

    BVR_ASSERT(bvri_create_texture_base(dest));

    // if texture view extension is enabled
    if(GLAD_GL_EXT_texture_view){
        glTextureViewEXT(
            dest->id, 
            dest->target,
            origin->id,
            origin->image.sformat,
            0, 1,
            layer, 1
        );
    }
    else {
        glTexStorage2D(GL_TEXTURE_2D, 1, origin->image.sformat, width, height);

        glCopyImageSubData(
            origin->id, origin->target,
            0, 0, 0, layer,
            dest->id, dest->target,
            0, 0, 0, 0, width, height, 1
        );

        glGenerateMipmap(dest->target);
    }

    glBindTexture(dest->target, 0);
    
    return BVR_OK;
}

int bvr_create_texture_from_image(bvr_texture_t* texture, bvr_image_t* image, int filter, int wrap){
    BVR_ASSERT(texture);
    BVR_ASSERT(image);

    texture->filter = filter;
    texture->wrap = wrap;
    texture->id = 0;
    texture->unit = 0;
    texture->target = GL_TEXTURE_2D;

    if(BVR_BUFFER_COUNT(image->layers) > 1){
        // TODO: compress images into one layer
    }

    bvri_create_texture_base(texture);

    glTexStorage2D(
        texture->target, 1, 
        texture->image.sformat, 
        image->width, image->height
    );

    glTexSubImage2D(
        texture->target, 0, 0, 0, 
        image->width, image->height, 
        image->format, 
        GL_UNSIGNED_BYTE, 
        image->pixels
    );

    glGenerateMipmap(texture->target);

    glBindTexture(texture->target, 0);

    free(image->pixels);
    image->pixels = NULL;

    return BVR_OK;
}

int bvr_create_texturef(bvr_texture_t* texture, FILE* file, int filter, int wrap){
    BVR_ASSERT(texture);
    BVR_ASSERT(file);

    bvr_create_imagef(&texture->image, file);
    if(!texture->image.pixels){
        BVR_PRINT("invalid image!");
        return BVR_FAILED;
    }

    return bvr_create_texture_from_image(texture, &texture->image, filter, wrap);    
}

void bvr_texture_enable(bvr_texture_t* texture){
    glActiveTexture(BVR_TEXTURE_UNIT0 + texture->unit);
    glBindTexture(texture->target, texture->id);
}

void bvr_texture_disable(bvr_texture_t* texture){
    glBindTexture(texture->target, 0);
}

void bvr_destroy_texture(bvr_texture_t* texture){
    BVR_ASSERT(texture);

    glDeleteTextures(1, &texture->id);
    
    bvr_destroy_image(&texture->image);
}

int bvr_create_texture_atlasf(bvr_texture_atlas_t* atlas, FILE* file, 
        uint32 tile_width, uint32 tile_height, int filter, int wrap){

    BVR_ASSERT(atlas);
    BVR_ASSERT(file);

    atlas->texture.filter = filter;
    atlas->texture.wrap = wrap;
    atlas->texture.target = GL_TEXTURE_2D_ARRAY;
    atlas->texture.id = 0;
    atlas->texture.unit = 0;

    atlas->tiles.width = tile_width;
    atlas->tiles.height = tile_height;

    bvr_create_imagef(&atlas->texture.image, file);
    if(!atlas->texture.image.pixels){
        BVR_PRINT("invalid image!");
        return BVR_FAILED;
    }

    if(BVR_BUFFER_COUNT(atlas->texture.image.layers) > 1){
        // TODO: compress images into one layer
    }

    const int tile_cx = atlas->texture.image.width / tile_width;
    const int tile_cy = atlas->texture.image.height / tile_height;

    atlas->tiles.count = tile_cx * tile_cy;

    bvri_create_texture_base(&atlas->texture);

    glTexStorage3D(
        atlas->texture.target, 1, 
        atlas->texture.image.sformat,
        atlas->tiles.width, atlas->tiles.height, 
        atlas->tiles.count
    );

    for (uint64 y = 0; y < atlas->texture.image.height; y += atlas->tiles.height)
    {
        for (uint64 x = 0; x < atlas->texture.image.width; x += atlas->tiles.width)
        {
#ifndef BVR_NO_FLIP
            glTexSubImage3D(
                GL_TEXTURE_2D_ARRAY, 0, 
                0, 
                0,
                atlas->tiles.count - ((y / atlas->tiles.height) * tile_cx + (tile_cx - x / atlas->tiles.width - 1)) - 1,
                atlas->tiles.width, 
                atlas->tiles.height, 
                1, atlas->texture.image.format, GL_UNSIGNED_BYTE,
                atlas->texture.image.pixels + (y * atlas->texture.image.width + x) * atlas->texture.image.channels
            );
#else
            glTexSubImage3D(
                GL_TEXTURE_2D_ARRAY, 0, 
                0, 
                0,
                ((y / atlas->tiles.height) * tile_cx + (x / atlas->tiles.width)),
                atlas->tiles.width, 
                atlas->tiles.height, 
                1, atlas->texture.image.format, GL_UNSIGNED_BYTE,
                atlas->texture.image.pixels + (y * atlas->texture.image.width + x) * atlas->texture.image.channels
            );
#endif
        }
        
    }
    
    glGenerateMipmap(atlas->texture.target);

    glBindTexture(atlas->texture.target, 0);
    
    free(atlas->texture.image.pixels);
    atlas->texture.image.pixels = NULL;

    return BVR_OK;
}

int bvr_create_layered_texturef(bvr_texture_t* texture, FILE* file, int filter, int wrap){
    BVR_ASSERT(texture);
    BVR_ASSERT(file);
    texture->filter = filter;
    texture->wrap = wrap;
    texture->target = GL_TEXTURE_2D_ARRAY;

    texture->id = 0;
    texture->unit = 0;

    bvr_create_imagef(&texture->image, file);
    if(!texture->image.pixels){
        BVR_PRINT("invalid image!");
        return BVR_FAILED;
    }

    if(texture->image.layers.size / sizeof(bvr_layer_t) < 1){
        BVR_PRINT("layered texture will load without layer info. Data might be lost.");
    }

    if(!texture->image.layers.data){
        bvri_create_empty_layer(&texture->image);
    }

    bvri_create_texture_base(texture);

    glTexStorage3D(
        texture->target, 1, 
        texture->image.sformat,
        texture->image.width, texture->image.height, 
        texture->image.layers.size / sizeof(bvr_layer_t)
    );

    for (uint64 layer = 0; layer < texture->image.layers.size / sizeof(bvr_layer_t); layer++)
    {

#ifndef BVR_NO_FLIP
        glTexSubImage3D(
            texture->target, 0, 
            0, 
            0,
            layer,
            texture->image.width, 
            texture->image.height, 
            1, texture->image.format, GL_UNSIGNED_BYTE,
            texture->image.pixels + texture->image.width * texture->image.height * texture->image.channels * layer
        );
#else
        glTexSubImage3D(
            texture->target, 0, 
            ((bvr_layer_t*)texture->image.layers.data)[layer].anchor_x, 
            ((bvr_layer_t*)texture->image.layers.data)[layer].anchor_y,
            layer,
            ((bvr_layer_t*)texture->image.layers.data)[layer].width, 
            ((bvr_layer_t*)texture->image.layers.data)[layer].height, 
            1, texture->image.format, GL_UNSIGNED_BYTE,
            texture->image.pixels + texture->image.width * texture->image.height * texture->image.channels * layer
        );
#endif
    }

    glGenerateMipmap(texture->target);
    
    glBindTexture(texture->target, 0);

    free(texture->image.pixels);
    texture->image.pixels = NULL;

    return BVR_OK;
}

int bvr_create_composite(bvr_composite_t* composite, bvr_image_t* target){
    BVR_ASSERT(composite);
    BVR_ASSERT(target);

    if(target->width <= 0 || target->height <= 0){
        BVR_PRINT("invalid image!");
        return BVR_OK;
    }

    composite->framebuffer = 0;
    composite->tex = 0;
    composite->image = target;

    glGenFramebuffers(1, &composite->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, composite->framebuffer);

    glGenTextures(1, &composite->tex);
    glBindTexture(GL_TEXTURE_2D, composite->tex);
    glTexImage2D(
        GL_TEXTURE_2D, 0, 
        GL_RGB, 
        composite->image->width, 
        composite->image->height, 
        0, GL_RGB, GL_UNSIGNED_BYTE, 
        NULL
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, composite->tex, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        BVR_PRINT("failed to create a new composite object!");
        return BVR_FAILED;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return BVR_OK;
}

void bvr_composite_enable(bvr_composite_t* composite){
    BVR_ASSERT(composite && composite->framebuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, composite->framebuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void bvr_composite_prepare(bvr_composite_t* composite){
    BVR_ASSERT(composite);

    glBindTexture(GL_TEXTURE_2D, composite->tex);
    glActiveTexture(GL_TEXTURE0);
}

void bvr_composite_disable(bvr_composite_t* composite){
    glBindFramebuffer(GL_FRAMEBUFFER, bvr_get_book_instance()->pipeline.render_target);
}

void bvr_destroy_composite(bvr_composite_t* composite){
    BVR_ASSERT(composite);

    glDeleteFramebuffers(1, &composite->framebuffer);
    glDeleteTextures(1, &composite->tex);

    composite->image = NULL;
}