#pragma once

#include <bvr/audio.h>
#include <bvr/window.h>
#include <bvr/graphics.h>

#include <bvr/collections/memstream.h>

#ifndef BVR_MAX_PAGE
    #define BVR_MAX_PAGE 8
#endif

#define BVR_INSTANCE() (bvr_create_book(NULL))
#define BVR_CAN_QUIT() (!BVR_INSTANCE()->window.awake)

typedef struct bvr_page_s {

} bvr_page_t;

struct bvr_page_slot_s {
    uint8 order;
    
    bool is_active;
    bool is_assigned;

    bvr_page_t page;
};

struct bvr_book_attributes_s {
    // the window width
    uint16 window_width;

    // the window width
    uint16 window_height;

    // the window flags
    int window_flags;

    // audio sample rate
    int sample_rate;

    // audio output channel count.
    // either BVR_AUDIO_MONO or BVR_AUDIO_STEREO
    uint8 channels;

    // garbage memorystream size
    size_t garbage_stream_size;

    // asset memorystream size
    size_t asset_stream_size;

    // book's name
    char* name;
};

/** 
 * @brief a book is an instance of an application.
 * a book will manage all game's main components such as 
 * the audio output or the window.
 */
typedef struct bvr_book_s {
    /// @brief game's audio mixer object
    bvr_audio_mixer_t mixer;

    /// @brief game's window object
    bvr_window_t window;
    
    /// @brief game's graphic pipeline
    bvr_pipeline_t graphics;

    /// @brief game's global variables and static components
    struct bvr_predefs predefs;

    struct bvr_page_slot_s slots[BVR_MAX_PAGE];

    struct bvr_memstream_s garbage;
    struct bvr_memstream_s assets; 
} bvr_book_t;

/**
 * @brief create a default game instance. If a book is already created, it will return the 
 * previously created instance.
 * 
 * @param book a preallocated book object that will be initialized.
 * @returns the pointer to initialized book objet.
 */
bvr_book_t* bvr_create_book(bvr_book_t* book);

/**
 * @brief prepare Beauvoir to render a new frame.
 * Clear all states and reset the pipeline.
 */
void bvr_new_frame(void);

/**
 * Push the current drawing queue to the screen. 
 * Force the drawing queue to be displayed.
 */
void bvr_flush(void);

/**
 * flush the draw queue and swap buffers.
 */
void bvr_render(void);

/**
 * @brief create a new game instance. If a book is already created, it will return the 
 * previously created instance.
 * 
 * @param book a preallocated book object that will be initialized.
 * @param attributes a collection of attributes to customize your book.
 * @returns the pointer to initialized book objet.
 */
bvr_book_t* bvr_create_book_attributes(bvr_book_t* book, struct bvr_book_attributes_s* attributes);

/**
 * @brief destroy a game instance.
 */
void bvr_destroy_book(bvr_book_t* book);