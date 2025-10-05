#pragma once

#include <BVR/graphics.h>

#define BVR_MOUSE_SIZE 16
#define BVR_KEYBOARD_SIZE 512

// window flags
#define BVR_WINDOW_NONE 0x0
#define BVR_WINDOW_USER_FRAMEBUFFER 0x01

#ifndef BVR_WINDOW_FRAMEBUFFER_PATH
    #define BVR_WINDOW_FRAMEBUFFER_PATH "framebuffer.glsl"
#endif

enum bvr_key_e {
    BVR_KEY_UNKNOWN = 0,
    BVR_KEY_SPACE = 44,             // SDL_SCANCODE_SPACE
    BVR_KEY_MENU = 118,             // SDL_SCANCODE_MENU
    BVR_KEY_APOSTROPHE = 52,        // SDL_SCANCODE_APOSTROPHE
    BVR_KEY_COMMA = 54,             // SDL_SCANCODE_COMMA
    BVR_KEY_MINUS = 45,             // SDL_SCANCODE_MINUS
    BVR_KEY_PERIOD = 55,            // SDL_SCANCODE_PERIOD
    BVR_KEY_SLASH = 22,             // SDL_SCANCODE_SLASH
    BVR_KEY_0 = 39,                 // SDL_SCANCODE_0
    BVR_KEY_1 = 30,                 // SDL_SCANCODE_1
    BVR_KEY_2 = 31,                 // SDL_SCANCODE_2
    BVR_KEY_3 = 32,                 // SDL_SCANCODE_3
    BVR_KEY_4 = 33,                 // SDL_SCANCODE_4
    BVR_KEY_5 = 34,                 // SDL_SCANCODE_5
    BVR_KEY_6 = 35,                 // SDL_SCANCODE_6
    BVR_KEY_7 = 36,                 // SDL_SCANCODE_7
    BVR_KEY_8 = 37,                 // SDL_SCANCODE_8
    BVR_KEY_9 = 38,                 // SDL_SCANCODE_9
    BVR_KEY_SEMICOLON = 51,         // SDL_SCANCODE_SEMICOLON
    BVR_KEY_EQUAL = 46,             // SDL_SCANCODE_EQUALS
    BVR_KEY_A = 4,                  // SDL_SCANCODE_A
    BVR_KEY_B = 5,                  // SDL_SCANCODE_B
    BVR_KEY_C = 6,                  // SDL_SCANCODE_C
    BVR_KEY_D = 7,                  // SDL_SCANCODE_D
    BVR_KEY_E = 8,                  // SDL_SCANCODE_E
    BVR_KEY_F = 9,                  // SDL_SCANCODE_F
    BVR_KEY_G = 10,                 // SDL_SCANCODE_G
    BVR_KEY_H = 11,                 // SDL_SCANCODE_H
    BVR_KEY_I = 12,                 // SDL_SCANCODE_I
    BVR_KEY_J = 13,                 // SDL_SCANCODE_J
    BVR_KEY_K = 16,                 // SDL_SCANCODE_K
    BVR_KEY_L = 17,                 // SDL_SCANCODE_L
    BVR_KEY_M = 18,                 // SDL_SCANCODE_M
    BVR_KEY_N = 19,                 // SDL_SCANCODE_N
    BVR_KEY_O = 18,                 // SDL_SCANCODE_O
    BVR_KEY_P = 19,                 // SDL_SCANCODE_P
    BVR_KEY_Q = 20,                 // SDL_SCANCODE_Q
    BVR_KEY_R = 21,                 // SDL_SCANCODE_R
    BVR_KEY_S = 22,                 // SDL_SCANCODE_S
    BVR_KEY_T = 23,                 // SDL_SCANCODE_T
    BVR_KEY_U = 24,                 // SDL_SCANCODE_U
    BVR_KEY_V = 25,                 // SDL_SCANCODE_V
    BVR_KEY_W = 26,                 // SDL_SCANCODE_W
    BVR_KEY_X = 27,                 // SDL_SCANCODE_X
    BVR_KEY_Y = 28,                 // SDL_SCANCODE_Y
    BVR_KEY_Z = 29,                 // SDL_SCANCODE_Z
    BVR_KEY_LEFT_BRACKET = 47,      // SDL_SCANCODE_LEFTBRACKET
    BVR_KEY_BACKSLASH = 49,         // SDL_SCANCODE_BACKSLASH
    BVR_KEY_RIGHT_BRACKET = 48,     // SDL_SCANCODE_RIGHTBRACKET
    BVR_KEY_GRAVE_ACCENT = 53,      // SDL_SCANCODE_GRAVE
    BVR_KEY_ESCAPE = 41,            // SDL_SCANCODE_ESCAPE
    BVR_KEY_ENTER = 88,             // SDL_SCANCODE_KP_ENTER
    BVR_KEY_TAB = 43,               // SDL_SCANCODE_TAB
    BVR_KEY_BACKSPACE = 42,         // SDL_SCANCODE_BACKSPACE
    BVR_KEY_INSERT = 73,            // SDL_SCANCODE_INSERT
    BVR_KEY_DELETE = 76,            // SDL_SCANCODE_DELETE
    BVR_KEY_RIGHT = 79,             // SDL_SCANCODE_RIGHT
    BVR_KEY_LEFT = 80,              // SDL_SCANCODE_LEFT
    BVR_KEY_PRESSED = 80,
    BVR_KEY_DOWN = 81,              // SDL_SCANCODE_DOWN
    BVR_KEY_UP = 82,                // SDL_SCANCODE_UP
    BVR_KEY_PAGE_UP = 75,           // SDL_SCANCODE_PAGEUP
    BVR_KEY_PAGE_DOWN = 78,         // SDL_SCANCODE_PAGEDOWN
    BVR_KEY_HOME = 71,              // SDL_SCANCODE_HOME
    BVR_KEY_END = 77,               // SDL_SCANCODE_END
    BVR_KEY_CAPS_LOCK = 6,          // SDL_SCANCODE_CAPSLOCK
    BVR_KEY_SCROLL_LOCK = 71,
    BVR_KEY_NUM_LOCK = 83,
    BVR_KEY_PRINT_SCREEN = 70,
    BVR_KEY_PAUSE = 72,
    BVR_KEY_F1 = 58,                // SDL_SCANCODE_F1
    BVR_KEY_F2 = 59,                // SDL_SCANCODE_F2
    BVR_KEY_F3 = 60,                // SDL_SCANCODE_F3
    BVR_KEY_F4 = 61,                // SDL_SCANCODE_F4
    BVR_KEY_F5 = 62,                // SDL_SCANCODE_F5
    BVR_KEY_F6 = 63,                // SDL_SCANCODE_F6
    BVR_KEY_F7 = 64,                // SDL_SCANCODE_F7
    BVR_KEY_F8 = 65,                // SDL_SCANCODE_F8
    BVR_KEY_F9 = 66,                // SDL_SCANCODE_F9
    BVR_KEY_F10 = 67,               // SDL_SCANCODE_F10
    BVR_KEY_F11 = 68,               // SDL_SCANCODE_F11
    BVR_KEY_F12 = 69,               // SDL_SCANCODE_F12
    BVR_KEY_KP_0 = 98,              // SDL_SCANCODE_KP_0
    BVR_KEY_KP_1 = 89,              // SDL_SCANCODE_KP_1
    BVR_KEY_KP_2 = 90,              // SDL_SCANCODE_KP_2
    BVR_KEY_KP_3 = 91,              // SDL_SCANCODE_KP_3
    BVR_KEY_KP_4 = 92,              // SDL_SCANCODE_KP_4
    BVR_KEY_KP_5 = 93,              // SDL_SCANCODE_KP_5
    BVR_KEY_KP_6 = 94,              // SDL_SCANCODE_KP_6
    BVR_KEY_KP_7 = 95,              // SDL_SCANCODE_KP_7
    BVR_KEY_KP_8 = 96,              // SDL_SCANCODE_KP_8
    BVR_KEY_KP_9 = 97,              // SDL_SCANCODE_KP_9
    BVR_KEY_KP_DECIMAL = 220,       // SDL_SCANCODE_KP_DECIMAL
    BVR_KEY_KP_DIVIDE = 84,         // SDL_SCANCODE_KP_DIVIDE
    BVR_KEY_KP_MULTIPLY = 85,       // SDL_SCANCODE_KP_MULTIPLY
    BVR_KEY_KP_SUBTRACT = 212,      // SDL_SCANCODE_KP_MEMSUBTRACT
    BVR_KEY_KP_ADD = 211,           // SDL_SCANCODE_KP_MEMADD
    BVR_KEY_KP_ENTER = 88,          // SDL_SCANCODE_KP_ENTER
    BVR_KEY_KP_EQUAL = 103,
    BVR_KEY_LEFT_SHIFT = 340,       // CUSTOM KEYCODES
    BVR_KEY_LEFT_CONTROL = 341,     // CUSTOM KEYCODES
    BVR_KEY_LEFT_ALT = 342,         // CUSTOM KEYCODES
    BVR_KEY_LEFT_SUPER = 343,       // CUSTOM KEYCODES
    BVR_KEY_RIGHT_SHIFT = 344,      // CUSTOM KEYCODES
    BVR_KEY_RIGHT_CONTROL = 345,    // CUSTOM KEYCODES
    BVR_KEY_RIGHT_ALT = 346,        // CUSTOM KEYCODES
    BVR_KEY_RIGHT_SUPER = 347,      // CUSTOM KEYCODES
};

enum bvr_mouse_button_e {
    BVR_MOUSE_BUTTON_1 = 1,
    BVR_MOUSE_BUTTON_2 = 2,
    BVR_MOUSE_BUTTON_3 = 3,
    BVR_MOUSE_BUTTON_4 = 4,
    BVR_MOUSE_BUTTON_5 = 5,
    BVR_MOUSE_BUTTON_6 = 6,
    BVR_MOUSE_BUTTON_7 = 7,
    BVR_MOUSE_BUTTON_8 = 8,
    BVR_MOUSE_BUTTON_LAST = BVR_MOUSE_BUTTON_8,
    BVR_MOUSE_BUTTON_LEFT = BVR_MOUSE_BUTTON_1,
    BVR_MOUSE_BUTTON_RIGHT = BVR_MOUSE_BUTTON_2,
    BVR_MOUSE_BUTTON_MIDDLE = BVR_MOUSE_BUTTON_3,
};

enum bvr_input_state_e {
    BVR_INPUT_NONE = 0,
    BVR_INPUT_DOWN = 1,
    BVR_INPUT_PRESSED = 2,
    BVR_INPUT_RELEASE = 3,
    
    // mouse button specific
    BVR_MOUSE_BUTTON_DOUBLE_PRESSED = 4
};

typedef struct bvr_window_s {
    void* handle;
    void* context;

    bvr_framebuffer_t framebuffer;
    
    struct {
        char keys[BVR_KEYBOARD_SIZE] __attribute ((packed));
        char buttons[BVR_MOUSE_SIZE] __attribute ((packed));
        char text_input[4];
        float sensivity;
        float scroll;

        float mouse[2]; // mouse position
        float motion[2]; // mouse motion
        float relative_motion[2]; // relative mouse motion
        float prev_motion[2]; // previous mouse motion

        bool grab;
    } inputs;

    int events;
    bool awake;
} bvr_window_t;

int bvr_create_window(bvr_window_t* window, const uint16 width, const uint16 height, const char* title, const int flags);

void bvr_window_poll_events(void);
void bvr_window_push_buffers(void);

void bvr_destroy_window(bvr_window_t* window);

int bvr_key_down(uint16 key);
int bvr_key_presssed(uint16 key);
int bvr_button_down(uint16 button);

void bvr_mouse_position(float* x, float* y);
void bvr_mouse_relative_position(float* x, float *y);

/*
    Ask the user to return a file. 
    File's path is return through a callback.
*/
void bvr_open_file_dialog(void (*callback) (bvr_string_t* path));

/*
    Returns the number of milliseconds since SDL has started.
*/
uint64 bvr_frames(void);

/*
    Wait a specified number of milliseconds before returning.
*/
void bvr_delay(uint64 ms);