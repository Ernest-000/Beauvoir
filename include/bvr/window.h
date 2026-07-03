#pragma once

#include <bvr/graphics.h>

#define BVR_MOUSE_SIZE 16
#define BVR_KEYBOARD_SIZE 512

// window flags
#define BVR_WINDOW_NONE 0x0
#define BVR_WINDOW_RESIZABLE 0x01
#define BVR_WINDOW_ALWAYS_ON_TOP 0x02
#define BVR_WINDOW_BORDERLESS 0x04
#define BVR_WINDOW_FULLSCREEN 0x08
#define BVR_WINDOW_CENTERED 0x10

#define BVR_WINDOW_USER_FRAMEBUFFER 0x11

#define BVR_WINDOW_DEFAULT (BVR_WINDOW_RESIZABLE | BVR_WINDOW_CENTERED) 

enum bvr_key_e {
    BVR_KEY_UNKNOWN = 0x00,
    BVR_KEY_A = 0x01,
    BVR_KEY_B = 0x02,
    BVR_KEY_C = 0x03,
    BVR_KEY_D = 0x04,
    BVR_KEY_E = 0x05,
    BVR_KEY_F = 0x06,
    BVR_KEY_G = 0x07,
    BVR_KEY_H = 0x08,
    BVR_KEY_I = 0x09,
    BVR_KEY_J = 0x0A,
    BVR_KEY_K = 0x0B,
    BVR_KEY_L = 0x0C,
    BVR_KEY_M = 0x0D,
    BVR_KEY_N = 0x0E,
    BVR_KEY_O = 0x0F,
    BVR_KEY_P = 0x10,
    BVR_KEY_Q = 0x11,
    BVR_KEY_R = 0x12,
    BVR_KEY_S = 0x13,
    BVR_KEY_T = 0x14,
    BVR_KEY_U = 0x15,
    BVR_KEY_V = 0x16,
    BVR_KEY_W = 0x17,
    BVR_KEY_X = 0x18,
    BVR_KEY_Y = 0x19,
    BVR_KEY_Z = 0x1A,
    BVR_KEY_0 = 0x1B,
    BVR_KEY_1 = 0x1C,
    BVR_KEY_2 = 0x1D,
    BVR_KEY_3 = 0x1E,
    BVR_KEY_4 = 0x1F,
    BVR_KEY_5 = 0x20,
    BVR_KEY_6 = 0x21,
    BVR_KEY_7 = 0x22,
    BVR_KEY_8 = 0x23,
    BVR_KEY_9 = 0x24,
    BVR_KEY_SPACE = 0x25,
    BVR_KEY_APOSTROPHE = 0x26,
    BVR_KEY_COMMA = 0x27,
    BVR_KEY_MINUS = 0x28,
    BVR_KEY_PERIOD = 0x29,
    BVR_KEY_SLASH = 0x2A,
    BVR_KEY_SEMICOLON = 0x2B,
    BVR_KEY_EQUAL = 0x2C,
    BVR_KEY_LEFT_BRACKET = 0x2D,
    BVR_KEY_BACKSLASH = 0x2E,
    BVR_KEY_RIGHT_BRACKET = 0x2F,
    BVR_KEY_GRAVE_ACCENT = 0x30,
    BVR_KEY_ESCAPE = 0x31,
    BVR_KEY_ENTER = 0x32,
    BVR_KEY_TAB = 0x33,
    BVR_KEY_BACKSPACE = 0x34,
    BVR_KEY_INSERT = 0x35,
    BVR_KEY_DELETE = 0x36,
    BVR_KEY_RIGHT = 0x37,
    BVR_KEY_LEFT = 0x38,
    BVR_KEY_DOWN = 0x39,
    BVR_KEY_UP = 0x3A,
    BVR_KEY_PAGE_UP = 0x3B,
    BVR_KEY_PAGE_DOWN = 0x3C,
    BVR_KEY_HOME = 0x3D,
    BVR_KEY_END = 0x3E,
    BVR_KEY_CAPS_LOCK = 0x3F,
    BVR_KEY_SCROLL_LOCK = 0x40,
    BVR_KEY_NUM_LOCK = 0x41,
    BVR_KEY_PRINT_SCREEN = 0x42,
    BVR_KEY_PAUSE = 0x43,
    BVR_KEY_MENU = 0x44,
    BVR_KEY_F1 = 0x45,
    BVR_KEY_F2 = 0x46,
    BVR_KEY_F3 = 0x47,
    BVR_KEY_F4 = 0x48,
    BVR_KEY_F5 = 0x49,
    BVR_KEY_F6 = 0x4A,
    BVR_KEY_F7 = 0x4B,
    BVR_KEY_F8 = 0x4C,
    BVR_KEY_F9 = 0x4D,
    BVR_KEY_F10 = 0x4E,
    BVR_KEY_F11 = 0x4F,
    BVR_KEY_F12 = 0x50,
    BVR_KEY_KP_0 = 0x51,
    BVR_KEY_KP_1 = 0x52,
    BVR_KEY_KP_2 = 0x53,
    BVR_KEY_KP_3 = 0x54,
    BVR_KEY_KP_4 = 0x55,
    BVR_KEY_KP_5 = 0x56,
    BVR_KEY_KP_6 = 0x57,
    BVR_KEY_KP_7 = 0x58,
    BVR_KEY_KP_8 = 0x59,
    BVR_KEY_KP_9 = 0x5A,
    BVR_KEY_KP_DECIMAL = 0x5B,
    BVR_KEY_KP_DIVIDE = 0x5C,
    BVR_KEY_KP_MULTIPLY = 0x5D,
    BVR_KEY_KP_SUBTRACT = 0x5E,
    BVR_KEY_KP_ADD = 0x5F,
    BVR_KEY_KP_ENTER = 0x60,
    BVR_KEY_KP_EQUAL = 0x61,

    BVR_KEY_LEFT_SHIFT = 0xF1,
    BVR_KEY_LEFT_CONTROL = 0xF2,
    BVR_KEY_LEFT_ALT = 0xF3,
    BVR_KEY_LEFT_SUPER = 0xF4,
    BVR_KEY_RIGHT_SHIFT = 0xF5,
    BVR_KEY_RIGHT_CONTROL = 0xF6,
    BVR_KEY_RIGHT_ALT = 0xF7,
    BVR_KEY_RIGHT_SUPER = 0xF8,
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
    BVR_MOUSE_BUTTON_RIGHT = BVR_MOUSE_BUTTON_3,
    BVR_MOUSE_BUTTON_MIDDLE = BVR_MOUSE_BUTTON_2,
};

enum bvr_input_state_e {
    BVR_INPUT_NONE = 0,
    BVR_INPUT_DOWN = 1,
    BVR_INPUT_PRESSED = 2,
    BVR_INPUT_RELEASE = 3,
    
    // mouse button specific
    BVR_MOUSE_BUTTON_DOUBLE_PRESSED = 4
};

typedef struct bvr_keyaxis_s {
    uint16 keys[2];
    uint16 alt_keys[2];
} bvr_keyaxis_t;

union bvr_window_handle_u {
    struct {
        void* wnd;
        void* gdi;
        void* hdc;
        void* ogl;
    } win32;

    struct {
        unsigned long xwindow; 
        unsigned long xcolormap;
        void* xdisplay;
        void* xvisual;

        unsigned long atoms[16];
    } x11;
};

typedef struct bvr_window_s {
    // window handle
    union bvr_window_handle_u handle;

    // ogl context
    void* context;

    uint16 width, height;
    uint16 x, y;

    int flags;
    int events;

    bool awake, focus;

    bvr_framebuffer_t framebuffer;
    
    struct {
        char keys[BVR_KEYBOARD_SIZE];
        char buttons[BVR_MOUSE_SIZE];
        char text_input[4];
        float sensivity;
        float scroll;

        // mouse position
        short mouse[2]; 

        // delta mouse position
        float motion[2]; 

        // relative to the window mouse motion
        float relative_motion[2];
        
        // previous mouse position
        short prev_mouse[2]; 

        bool grab;

        // default key axis
        struct {
            bvr_keyaxis_t horizontal;
            bvr_keyaxis_t vertical;
        } axis;
    } inputs;

    struct bvr_chono_s {
        unsigned long long initial_frame;
        uint64 current_time, previous_time;
        float delta_time;
    } timer;

    // store device informations
    struct {
        char version[32];
        char name[32];

        char gl_version[32];
        char glsl_version[32];
    } vendor;
} bvr_window_t;

int bvr_create_window(bvr_window_t* window, const uint16 width, const uint16 height, const char* title, const int flags);

void bvr_window_poll_events(bvr_window_t* window);
void bvr_window_push_buffers(bvr_window_t* window);

void bvr_window_set_size(bvr_window_t* window, const uint16 width, const uint16 height);
void bvr_window_set_position(bvr_window_t* window, const uint16 x, const uint16 y);

void bvr_destroy_window(bvr_window_t* window);

int bvr_key_down(uint16 key);
int bvr_key_presssed(uint16 key);

int bvr_axis_down(bvr_keyaxis_t* axis);
int bvr_axis_presssed(bvr_keyaxis_t* axis);

int bvr_button_down(uint16 button);
int bvr_button_pressed(uint16 button);

void bvr_mouse_position(float* x, float* y);
void bvr_mouse_relative_position(float* x, float* y);

float bvr_mouse_scroll();

/*
    Ask the user to return a file. 
    File's path is return through a callback.
*/
void bvr_open_file_dialog(void (*callback) (bvr_string_t* path), const char* filters, bool allow_multiple);

/*
    Returns the number of milliseconds since SDL has started.
*/
uint64 bvr_get_frame(void);

/*
    Return the time in ms between two frames
*/
float bvr_get_delta_time(void);

/*
    Wait a specified number of milliseconds before returning.
*/
void bvr_delay(uint64 ms);