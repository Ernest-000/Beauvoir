#include <bvr/window.h>

#include <bvr/common.h>
#include <bvr/book.h>
#include <bvr/gl.h>

#include <string.h>
#include <memory.h>

#ifdef _WIN32
    #include <windows.h>
    #include <windowsx.h>
    #include <gl\gl.h>
    #include <gl\glu.h>
    #include <gl\glaux.h>
#elif __unix__
    #include <unistd.h>
    #include <time.h>
    #include <sys/time.h>

    #include <X11/Xlib.h>

    // make sure to don't import ogl multiple times
    #define __gl_h_ 
    #include <GL/glx.h>
#endif

// current instanced window
static bvr_window_t* __window = NULL;
static uint8 __keycodes[0xffff];

static void* bvri_load_proc(const char* name);

// window behaviour function declaration promises

// create a new window
static int bvri_create_window_impl(bvr_window_t* window, const uint16 width, const uint16 height, const char* title, const int flags);

// pump window's events
static void bvri_window_poll_events_impl(bvr_window_t* window);

// swap buffers
static void bvri_window_push_buffers_impl(bvr_window_t* window);

// window set size
static void bvr_window_set_size_impl(bvr_window_t* window, const uint16 width, const uint16 height);

// window set position
static void bvr_window_set_position_impl(bvr_window_t* window, const uint16 x, const uint16 y);

// window set name
static void bvr_window_set_name_impl(bvr_window_t* window, const char* name);

// window destroy
static void bvri_window_destroy_impl(bvr_window_t* window);

// os timer
static uint64 bvri_get_ns_tick_impl();

// os sleep for x nanoseconds
static void bvri_thread_wait_ns(uint64 ns);

// register and create the current keyboard layout
static void bvri_create_keymap_layout();

void static bvr_error_callback(GLenum source, GLenum type, GLuint id, 
    GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

#ifdef _WIN32

// windproc callback
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// main windows create function for win32
static int bvri_create_window_impl(bvr_window_t* window, const uint16 width, const uint16 height, const char* title, const int flags){

    PIXELFORMATDESCRIPTOR pfd;
    WNDCLASS win;
    void* hInstance = NULL;

    const int buffer_bits = 24;
    DWORD sMode = SW_SHOW;

    hInstance = GetModuleHandle(NULL);
    window->handle.win32.gdi = NULL;
    window->handle.win32.hdc = NULL;
    window->handle.win32.wnd = NULL;
    window->handle.win32.ogl = NULL;

    win.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    win.lpfnWndProc = (WNDPROC)WndProc;
    win.cbClsExtra = false;
    win.cbWndExtra = false;
    win.hInstance = hInstance;

    // change icon here
    win.hIcon = LoadIcon(NULL, IDI_WINLOGO);

    // change cursor here
    win.hCursor = LoadCursor(NULL, IDC_ARROW);

    win.hbrBackground = NULL;
    win.lpszMenuName = NULL;
    win.lpszClassName = BVR_CLASS_NAME;

    int wx = 0;
    int wy = 0;
    int wwidth = width;
    int wheight = height;
    
    BVR_ASSERT(RegisterClass(&win));

    // here create flags
    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    DWORD dwStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    
    if(BVR_HAS_FLAG(flags, BVR_WINDOW_RESIZABLE)){
        dwStyle |= WS_THICKFRAME;
    }

    if(BVR_HAS_FLAG(flags, BVR_WINDOW_BORDERLESS)){
        dwStyle ^= (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_BORDER);
    }

    if(BVR_HAS_FLAG(flags, BVR_WINDOW_ALWAYS_ON_TOP)){
        dwExStyle |= WS_EX_TOPMOST;
    }

    if(BVR_HAS_FLAG(flags, BVR_WINDOW_FULLSCREEN)){
        HMONITOR monitor = MonitorFromWindow(GetForegroundWindow(), MONITOR_DEFAULTTONEAREST);
        BVR_ASSERT(monitor);

        MONITORINFO minfo = {sizeof(minfo)};
        BVR_ASSERT(GetMonitorInfo(monitor, &minfo));

        wx = minfo.rcMonitor.left;
        wy = minfo.rcMonitor.top;
        wwidth = minfo.rcMonitor.right - minfo.rcMonitor.left;
        wheight = minfo.rcMonitor.bottom - minfo.rcMonitor.top;

        dwExStyle |= WS_EX_TOPMOST;

        sMode = SW_SHOWMAXIMIZED;
    }

    if(BVR_HAS_FLAG(flags, BVR_WINDOW_CENTERED)){
        HMONITOR monitor = MonitorFromWindow(GetForegroundWindow(), MONITOR_DEFAULTTONEAREST);
        BVR_ASSERT(monitor);

        MONITORINFO minfo = {sizeof(minfo)};
        BVR_ASSERT(GetMonitorInfo(monitor, &minfo));
        
        int monitor_width = minfo.rcMonitor.right - minfo.rcMonitor.left;
        int monitor_height = minfo.rcMonitor.bottom - minfo.rcMonitor.top;
        
        wx = minfo.rcMonitor.left + monitor_width / 2 - width / 2;
        wy = minfo.rcMonitor.top + monitor_height / 2 - height / 2;
    }

    window->handle.win32.wnd = CreateWindowExA(
        dwExStyle, BVR_CLASS_NAME, title,
        dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        wx, wy, wwidth, wheight, NULL, NULL, hInstance, NULL
    );

    // overwrite window's properties to adapt to flags
    window->width = wwidth;
    window->height = wheight;
    window->x = wx;
    window->y = wy;

    BVR_ASSERT(window->handle.win32.wnd);

    // setup pixel format
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = buffer_bits;
    pfd.cRedBits = 0;
    pfd.cRedShift = 0;
    pfd.cGreenBits = 0;
    pfd.cGreenShift = 0;
    pfd.cBlueBits= 0;
    pfd.cBlueShift = 0;
    pfd.cAlphaBits = 0;
    pfd.cAlphaShift = 0;

    pfd.cAccumBits = 0;
    pfd.cAccumRedBits = 0;
    pfd.cAccumGreenBits = 0;
    pfd.cAccumBlueBits = 0;
    pfd.cAccumAlphaBits = 0;

    // depth buffer is 24 bits
    pfd.cDepthBits = buffer_bits;

    // stencil buffer is 8 bits
    // maybe remove?
    pfd.cStencilBits = 8;

    pfd.cAuxBuffers = 0;

    pfd.iLayerType = PFD_MAIN_PLANE;
    pfd.bReserved = 0;
    pfd.dwLayerMask = 0;
    pfd.dwVisibleMask= 0;
    pfd.dwDamageMask = 0;

    // create the private gdi context
    window->handle.win32.hdc = GetDC(window->handle.win32.wnd);
    BVR_ASSERT(window->handle.win32.hdc);

    int pformat = ChoosePixelFormat(window->handle.win32.hdc, &pfd);
    BVR_ASSERT(pformat);

    BVR_ASSERT(SetPixelFormat(window->handle.win32.hdc, pformat, &pfd));

    // load ogl dll
    window->handle.win32.ogl = LoadLibraryA("opengl32.dll");
    BVR_ASSERT(window->handle.win32.ogl);

    // load wgl
    window->context = wglCreateContext(window->handle.win32.hdc);
    BVR_ASSERT(wglMakeCurrent(window->handle.win32.hdc, window->context));

    ShowWindow(window->handle.win32.wnd, sMode);

    return 1;
}

static void bvri_window_poll_events_impl(bvr_window_t* window){
    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

static void bvri_window_push_buffers_impl(bvr_window_t* window){
    SwapBuffers(window->handle.win32.hdc);
}

static void bvr_window_set_size_impl(bvr_window_t* window, const uint16 width, const uint16 height){
    SetWindowPos(window->handle.win32.wnd,
        0, window->x, window->y, width, height,
        SWP_NOREPOSITION
    );
}

static void bvr_window_set_position_impl(bvr_window_t* window, const uint16 x, const uint16 y){
    SetWindowPos(window->handle.win32.wnd,
        0, x, y, window->width, window->height,
        SWP_NOSIZE
    );
}

static void bvr_window_set_name_impl(bvr_window_t* window, const char* name){
    SetWindowTextA(window->handle.win32.wnd, name);
}

static void bvri_window_destroy_impl(bvr_window_t* window){
    if(window->context){
        wglDeleteContext(window->context);
        FreeLibrary(window->handle.win32.ogl);
    }
    
    if(window->handle.win32.wnd){
        DestroyWindow(window->handle.win32.wnd);
    }
    
    window->handle.win32.gdi = NULL;
    window->handle.win32.hdc = NULL;
    window->handle.win32.wnd = NULL;
    window->handle.win32.ogl = NULL;
    window->context = NULL;
}

// load windows's virtual scancode layout to bind it
// to beauvoir's layout :]
static void bvri_create_keycode_layout(){

    // NULL key
    __keycodes[0] = BVR_KEY_UNKNOWN;

    // numbers
    __keycodes[0x30] = BVR_KEY_0;
    __keycodes[0x31] = BVR_KEY_1;
    __keycodes[0x32] = BVR_KEY_2;
    __keycodes[0x33] = BVR_KEY_3;
    __keycodes[0x34] = BVR_KEY_4;
    __keycodes[0x35] = BVR_KEY_5;
    __keycodes[0x36] = BVR_KEY_6;
    __keycodes[0x37] = BVR_KEY_7;
    __keycodes[0x38] = BVR_KEY_8;
    __keycodes[0x39] = BVR_KEY_9;
    
    // letters
    __keycodes[0x41] = BVR_KEY_A;
    __keycodes[0x42] = BVR_KEY_B;
    __keycodes[0x43] = BVR_KEY_C;
    __keycodes[0x44] = BVR_KEY_D;
    __keycodes[0x45] = BVR_KEY_E;
    __keycodes[0x46] = BVR_KEY_F;
    __keycodes[0x47] = BVR_KEY_G;
    __keycodes[0x48] = BVR_KEY_H;
    __keycodes[0x49] = BVR_KEY_I;
    __keycodes[0x4A] = BVR_KEY_J;
    __keycodes[0x4B] = BVR_KEY_K;
    __keycodes[0x4C] = BVR_KEY_L;
    __keycodes[0x4D] = BVR_KEY_M;
    __keycodes[0x4E] = BVR_KEY_N;
    __keycodes[0x4F] = BVR_KEY_O;
    __keycodes[0x50] = BVR_KEY_P;
    __keycodes[0x51] = BVR_KEY_Q;
    __keycodes[0x52] = BVR_KEY_R;
    __keycodes[0x53] = BVR_KEY_S;
    __keycodes[0x54] = BVR_KEY_T;
    __keycodes[0x55] = BVR_KEY_U;
    __keycodes[0x56] = BVR_KEY_V;
    __keycodes[0x57] = BVR_KEY_W;
    __keycodes[0x58] = BVR_KEY_X;
    __keycodes[0x59] = BVR_KEY_Y;
    __keycodes[0x5A] = BVR_KEY_Z;
    
    __keycodes[VK_OEM_1] = BVR_KEY_SEMICOLON;
    __keycodes[VK_OEM_2] = BVR_KEY_SLASH;
    __keycodes[VK_OEM_3] = BVR_KEY_GRAVE_ACCENT;
    __keycodes[VK_OEM_4] = BVR_KEY_LEFT_BRACKET;
    __keycodes[VK_OEM_5] = BVR_KEY_BACKSLASH;
    __keycodes[VK_OEM_6] = BVR_KEY_RIGHT_BRACKET;
    __keycodes[VK_OEM_7] = BVR_KEY_APOSTROPHE;
    __keycodes[VK_OEM_COMMA] = BVR_KEY_COMMA;
    __keycodes[VK_OEM_MINUS] = BVR_KEY_MINUS;
    __keycodes[VK_OEM_PERIOD] = BVR_KEY_PERIOD;
    __keycodes[VK_OEM_PLUS] = BVR_KEY_EQUAL;

    // system keys
    __keycodes[VK_SPACE] = BVR_KEY_SPACE;
    __keycodes[VK_MENU] = BVR_KEY_MENU;
    __keycodes[VK_ESCAPE] = BVR_KEY_ESCAPE;
    __keycodes[VK_RETURN] = BVR_KEY_ENTER;
    __keycodes[VK_TAB] = BVR_KEY_TAB;
    __keycodes[VK_BACK] = BVR_KEY_BACKSPACE;
    __keycodes[VK_INSERT] = BVR_KEY_INSERT;
    __keycodes[VK_DELETE] = BVR_KEY_DELETE;
    __keycodes[VK_RIGHT] = BVR_KEY_RIGHT;
    __keycodes[VK_LEFT] = BVR_KEY_LEFT;
    __keycodes[VK_DOWN] = BVR_KEY_DOWN;
    __keycodes[VK_UP] = BVR_KEY_UP;
    __keycodes[VK_PRIOR] = BVR_KEY_PAGE_UP;
    __keycodes[VK_NEXT] = BVR_KEY_PAGE_DOWN;
    __keycodes[VK_HOME] = BVR_KEY_HOME;
    __keycodes[VK_END] = BVR_KEY_END;
    
    // system
    __keycodes[VK_CAPITAL] = BVR_KEY_CAPS_LOCK;
    __keycodes[VK_SCROLL] = BVR_KEY_SCROLL_LOCK;
    __keycodes[VK_NUMLOCK] = BVR_KEY_NUM_LOCK;
    __keycodes[VK_PRINT] = BVR_KEY_PRINT_SCREEN;
    __keycodes[VK_PAUSE] = BVR_KEY_PAUSE;

    // function
    __keycodes[VK_F1] = BVR_KEY_F1;
    __keycodes[VK_F2] = BVR_KEY_F2;
    __keycodes[VK_F3] = BVR_KEY_F3;
    __keycodes[VK_F4] = BVR_KEY_F4;
    __keycodes[VK_F5] = BVR_KEY_F5;
    __keycodes[VK_F6] = BVR_KEY_F6;
    __keycodes[VK_F7] = BVR_KEY_F7;
    __keycodes[VK_F8] = BVR_KEY_F8;
    __keycodes[VK_F9] = BVR_KEY_F9;
    __keycodes[VK_F10] = BVR_KEY_F10;
    __keycodes[VK_F11] = BVR_KEY_F11;
    __keycodes[VK_F12] = BVR_KEY_F12;

    // keypad
    __keycodes[VK_NUMPAD0] = BVR_KEY_KP_0;
    __keycodes[VK_NUMPAD1] = BVR_KEY_KP_1;
    __keycodes[VK_NUMPAD2] = BVR_KEY_KP_2;
    __keycodes[VK_NUMPAD3] = BVR_KEY_KP_3;
    __keycodes[VK_NUMPAD4] = BVR_KEY_KP_4;
    __keycodes[VK_NUMPAD5] = BVR_KEY_KP_5;
    __keycodes[VK_NUMPAD6] = BVR_KEY_KP_6;
    __keycodes[VK_NUMPAD7] = BVR_KEY_KP_7;
    __keycodes[VK_NUMPAD8] = BVR_KEY_KP_8;
    __keycodes[VK_NUMPAD9] = BVR_KEY_KP_9;
    __keycodes[VK_DECIMAL] = BVR_KEY_KP_DECIMAL;
    __keycodes[VK_DIVIDE] = BVR_KEY_KP_DIVIDE;
    __keycodes[VK_MULTIPLY] = BVR_KEY_KP_MULTIPLY;
    __keycodes[VK_SUBTRACT] = BVR_KEY_KP_SUBTRACT;
    __keycodes[VK_ADD] = BVR_KEY_KP_ADD;
    __keycodes[VK_RETURN] = BVR_KEY_KP_ENTER;
    __keycodes[VK_OEM_PLUS] = BVR_KEY_KP_EQUAL;

    // modifiers
    __keycodes[VK_CONTROL] = BVR_KEY_LEFT_CONTROL;
    __keycodes[VK_SHIFT] = BVR_KEY_LEFT_SHIFT;
    __keycodes[VK_LMENU] = BVR_KEY_LEFT_ALT;

}

// callback plz
// src: https://github.com/glfw/glfw/blob/master/src/win32_window.c
LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam){
    
    // __win guard
    if(__window == NULL || wglGetCurrentContext() == NULL){
        return DefWindowProc(wnd, msg, wParam, lParam);
    }

    switch (msg)
    {
    // window activate message
    case WM_ACTIVATE:
        __window->focus = !HIWORD(wParam);
        return 0;
    
    case WM_SYSCOMMAND:
        switch (wParam)
        {
        case SC_SCREENSAVE: return 0;
        case SC_MONITORPOWER: return 0;
        default: break;
        }

        break;
    
    case WM_CLOSE:
        __window->awake = 0;
        return 0;
        
    // destroy event is no-op
    // because we handle window destroying ourself
    case WM_DESTROY:
        return 0;
    
    // do keys
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP: 
        {
            int action = (HIWORD(lParam) & KF_UP) ? BVR_INPUT_RELEASE : BVR_INPUT_PRESSED;
            int key = __keycodes[LOWORD(wParam)];

            // on repeat
            if(__window->inputs.keys[key] == BVR_INPUT_PRESSED){
                action = BVR_INPUT_DOWN;
            }

            // controll handling
            if(wParam == VK_CONTROL){
                if(HIWORD(lParam) & KF_EXTENDED){
                    key = BVR_KEY_RIGHT_CONTROL;
                }
            }

            __window->inputs.keys[key] = action;
        }
        break;
    
    // do window resizing
    case WM_SIZE:
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            bool iconified = wParam == SIZE_MINIMIZED;
            bool maxmized = wParam == SIZE_MAXIMIZED;

            if(__window->awake && __window->context){
                __window->width = width;
                __window->height = height;

                if(width > 0 && height > 0){
                    glViewport(0, 0, width, height);
                }
            }
        }
        break;
    
    // do key layout
    case WM_INPUTLANGCHANGE: break;

    // do char buffer
    case WM_CHAR: 
    case WM_SYSCHAR: 
        break;
    
    // do char buffer but unicode
    case WM_UNICHAR:
        break;
    
    // do mouse movements
    case WM_MOUSEMOVE:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            __window->inputs.mouse[0] = x;
            __window->inputs.mouse[1] = y;
            __window->inputs.prev_mouse[0] = __window->inputs.mouse[0];
            __window->inputs.prev_mouse[1] = __window->inputs.mouse[1];
            __window->inputs.motion[0] = __window->inputs.prev_mouse[0] -  __window->inputs.mouse[0];
            __window->inputs.motion[0] = __window->inputs.prev_mouse[1] -  __window->inputs.mouse[1];
            __window->inputs.relative_motion[0] = __window->inputs.motion[0];
            __window->inputs.relative_motion[1] = __window->inputs.motion[1];
        }
        break;

    // do mouse button
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
        {
            int action = BVR_INPUT_RELEASE;
            int button = 0;
            
            // find the currect button
            if(msg == WM_LBUTTONDOWN || WM_LBUTTONUP) button = BVR_MOUSE_BUTTON_LEFT;
            else if(msg == WM_RBUTTONDOWN || WM_RBUTTONUP) button = BVR_MOUSE_BUTTON_RIGHT;
            else if(msg == WM_MBUTTONDOWN || WM_MBUTTONUP) button = BVR_MOUSE_BUTTON_MIDDLE;
            else if(GET_XBUTTON_WPARAM(wParam) == XBUTTON1) button = BVR_MOUSE_BUTTON_4;
            else button = BVR_MOUSE_BUTTON_5;

            if(msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN){
                action = BVR_INPUT_PRESSED;
            }

            if(__window->inputs.buttons[button] == BVR_INPUT_PRESSED){
                action = BVR_INPUT_DOWN;
            }

            __window->inputs.buttons[button] = action;
        }
        break;

    // do y scroll
    case WM_MOUSEWHEEL:
        __window->inputs.scroll = (float)((short)HIWORD(wParam)/ (float)WHEEL_DELTA);
        break;

    case WM_MOUSEHWHEEL:
        // no-op for w mouse wheel
        break;

    // do window moving
    case WM_MOVE:
        __window->x = GET_X_LPARAM(lParam);
        __window->y = GET_Y_LPARAM(lParam);
        break;

    default:
        break;
    }

    return DefWindowProc(wnd, msg, wParam, lParam);
}

// win32 load proc function callback
static void* bvri_load_proc(const char* name){
    void* proc = wglGetProcAddress(name);

    // when wglGetProcAdress cannot find the functions
    if(proc == NULL || proc == (void*)0x1 || proc == (void*)0x2 || proc == (void*)0x3 || proc == (void*)-1){
        // fallback on opengl32 for core functions
        proc = (void*)GetProcAddress(__window->handle.win32.ogl, name);
    }

    return proc;
}

/*
https://github.com/ThomasHabets/monotonic_clock/blob/master/src/monotonic_win32.c
*/
static uint64 bvri_get_tick_impl(){
    static uint64 scale_factor;

	LARGE_INTEGER count;
	BOOL ret = QueryPerformanceCounter(&count);

	if (scale_factor == 0) {
		LARGE_INTEGER frequency;
		BOOL ret = QueryPerformanceFrequency(&frequency);
		scale_factor = frequency.QuadPart;
	}

	return count.QuadPart / scale_factor;
}

static void bvri_thread_wait(uint64 ns){
    BVR_ASSERT(__window->handle.win32.wnd)
    
    DWORD delay = (DWORD)(ns / 1000000UL);
    HANDLE event = __window->handle.win32.wnd;
    
    WaitForSingleObjectEx(event, delay, false);
}

#elif __unix__

#define BVRI_WM_ATOM_DELETE 0
#define BVRI_WM_ATOM_NAME 1
#define BVRI_WM_ATOM_ICON_NAME 2

#define BVRI_WM_ATOM_UTF8 10

/*
https://github.com/gamedevtech/X11OpenGLWindow
https://github.com/glfw/glfw/blob/master/src/x11_window.c
*/
static int bvri_create_window_impl(bvr_window_t* window, const uint16 width, const uint16 height, const char* title, const int flags){
    
    uint32 wx = 0;
    uint32 wy = 0;
    uint32 wwidth = width;
    uint32 wheight = height;

    uint32 border_width = 0;

    int screen_id = 0;
    Screen* screen = NULL;

    GLXFBConfig fbc;
    GLXFBConfig* fbc_list = NULL;

    Atom atomWmeDeleteWin;
    XSetWindowAttributes win_attribs;

    // open the default display
    window->handle.x11.xdisplay = XOpenDisplay(NULL);
    BVR_ASSERT(window->handle.x11.xdisplay);

    screen = DefaultScreenOfDisplay(window->handle.x11.xdisplay);
    screen_id = DefaultScreen(window->handle.x11.xdisplay);

    // constant opengl x attributes
    const int glx_attributes[] = {
        GLX_X_RENDERABLE, true,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_DOUBLEBUFFER, true,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_SAMPLE_BUFFERS, 0,
        GLX_SAMPLES, 0,
        None 
    };

    // constant opengl attributes
    const int ogl_attributes[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 2,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		None
	};

    // find the best frame buffer configuration
    {
        int fbc_count;
        fbc_list = glXChooseFBConfig(window->handle.x11.xdisplay, screen_id, glx_attributes, &fbc_count);
        
        BVR_ASSERT(fbc_list);
        BVR_ASSERT(fbc_count > 0);

        int best_fbci = -1;
        int best_num_samp = -1;
        
        for (int i = 0; i < fbc_count; ++i)
        {
            XVisualInfo* vi = glXGetVisualFromFBConfig(window->handle.x11.xdisplay, fbc_list[i]);
            
            // check for unusable visual
            if(vi == NULL){
                continue;
            }

            int samp_buf, samples;
            glXGetFBConfigAttrib(window->handle.x11.xdisplay, fbc_list[i], GLX_SAMPLE_BUFFERS, &samp_buf);
            glXGetFBConfigAttrib(window->handle.x11.xdisplay, fbc_list[i], GLX_SAMPLES, &samples);
            
            XFree(vi);
            
            if (best_fbci < 0 || (samp_buf && samples > best_num_samp)){
                best_fbci = i;
                best_num_samp = samples;
            }

        }

        BVR_ASSERT(best_fbci >= 0);

        fbc = fbc_list[best_fbci];
        XFree(fbc_list);
    }
    

    window->handle.x11.xvisual = glXGetVisualFromFBConfig(window->handle.x11.xdisplay, fbc);
    BVR_ASSERT(window->handle.x11.xvisual);

    window->handle.x11.xcolormap = XCreateColormap(
        window->handle.x11.xdisplay,
        RootWindow(window->handle.x11.xdisplay, screen_id),
        ((XVisualInfo*)window->handle.x11.xvisual)->visual,
        AllocNone
    );

    win_attribs.border_pixel = BlackPixel(window->handle.x11.xdisplay, screen_id);
    win_attribs.background_pixel = WhitePixel(window->handle.x11.xdisplay, screen_id);
    win_attribs.override_redirect = true;
    win_attribs.event_mask = ExposureMask;
    win_attribs.colormap = window->handle.x11.xcolormap;

    window->handle.x11.xwindow = XCreateWindow(
        window->handle.x11.xdisplay,
        RootWindow(window->handle.x11.xdisplay, screen_id),
        wx, wy, wwidth, wheight, border_width,
        ((XVisualInfo*)window->handle.x11.xvisual)->depth,
        InputOutput, ((XVisualInfo*)window->handle.x11.xvisual)->visual,
        CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &win_attribs
    );
    BVR_ASSERT(window->handle.x11.xwindow);

    // register atoms
    window->handle.x11.atoms[BVRI_WM_ATOM_DELETE] = XInternAtom(window->handle.x11.xdisplay, "WM_DELETE_WINDOW", false);
    window->handle.x11.atoms[BVRI_WM_ATOM_NAME] = XInternAtom(window->handle.x11.xdisplay, "WM_NAME", false);
    window->handle.x11.atoms[BVRI_WM_ATOM_ICON_NAME] = XInternAtom(window->handle.x11.xdisplay, "WM_ICON_NAME", false);

    window->handle.x11.atoms[BVRI_WM_ATOM_UTF8] = XInternAtom(window->handle.x11.xdisplay, "UTF8_STRING", false);

    // custom closing method
    XSetWMProtocols(
        window->handle.x11.xdisplay, 
        window->handle.x11.xwindow, 
        &window->handle.x11.atoms[BVRI_WM_ATOM_DELETE], 1
    );

    window->context = glXCreateNewContext(
        window->handle.x11.xdisplay,
        fbc, GLX_RGBA_TYPE, 0, true
    );

    BVR_ASSERT(window->context);
    BVR_ASSERT(glXMakeCurrent(
        window->handle.x11.xdisplay,
        window->handle.x11.xwindow,
        window->context
    ));

    XClearWindow(window->handle.x11.xdisplay, window->handle.x11.xwindow);

    // input mask
    XSelectInput(
        window->handle.x11.xdisplay, 
        window->handle.x11.xwindow, 
        KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | 
        ButtonMotionMask | StructureNotifyMask | FocusChangeMask | EnterWindowMask | 
        LeaveWindowMask | PropertyChangeMask
    );

    // update title
    bvr_window_set_name_impl(window, title);

    // flush all modifies properties
    XFlush(window->handle.x11.xdisplay);    
    
    // show window
    XMapRaised(window->handle.x11.xdisplay, window->handle.x11.xwindow);
}

static void bvri_create_keymap_layout(){
    __keycodes[0] = BVR_KEY_UNKNOWN;
    
    // numbers
    __keycodes[XK_0] = BVR_KEY_0; 
    __keycodes[XK_1] = BVR_KEY_1; 
    __keycodes[XK_2] = BVR_KEY_2; 
    __keycodes[XK_3] = BVR_KEY_3; 
    __keycodes[XK_4] = BVR_KEY_4; 
    __keycodes[XK_5] = BVR_KEY_5; 
    __keycodes[XK_6] = BVR_KEY_6; 
    __keycodes[XK_7] = BVR_KEY_7; 
    __keycodes[XK_8] = BVR_KEY_8; 
    __keycodes[XK_9] = BVR_KEY_9;
    
    // letters
    __keycodes[XK_A] = BVR_KEY_A; 
    __keycodes[XK_B] = BVR_KEY_B; 
    __keycodes[XK_C] = BVR_KEY_C; 
    __keycodes[XK_D] = BVR_KEY_D; 
    __keycodes[XK_E] = BVR_KEY_E; 
    __keycodes[XK_F] = BVR_KEY_F; 
    __keycodes[XK_G] = BVR_KEY_G; 
    __keycodes[XK_H] = BVR_KEY_H; 
    __keycodes[XK_I] = BVR_KEY_I; 
    __keycodes[XK_J] = BVR_KEY_J; 
    __keycodes[XK_K] = BVR_KEY_K; 
    __keycodes[XK_L] = BVR_KEY_L; 
    __keycodes[XK_M] = BVR_KEY_M; 
    __keycodes[XK_N] = BVR_KEY_N; 
    __keycodes[XK_O] = BVR_KEY_O; 
    __keycodes[XK_P] = BVR_KEY_P; 
    __keycodes[XK_Q] = BVR_KEY_Q; 
    __keycodes[XK_R] = BVR_KEY_R; 
    __keycodes[XK_S] = BVR_KEY_S; 
    __keycodes[XK_T] = BVR_KEY_T; 
    __keycodes[XK_U] = BVR_KEY_U; 
    __keycodes[XK_V] = BVR_KEY_V; 
    __keycodes[XK_W] = BVR_KEY_W; 
    __keycodes[XK_X] = BVR_KEY_X; 
    __keycodes[XK_Y] = BVR_KEY_Y; 
    __keycodes[XK_Z] = BVR_KEY_Z; 

    // system
    __keycodes[XK_semicolon] = BVR_KEY_SEMICOLON;
    __keycodes[XK_slash] = BVR_KEY_SLASH;
    __keycodes[XK_grave] = BVR_KEY_GRAVE_ACCENT;
    __keycodes[XK_bracketleft] = BVR_KEY_LEFT_BRACKET;
    __keycodes[XK_backslash] = BVR_KEY_BACKSLASH;
    __keycodes[XK_bracketright] = BVR_KEY_RIGHT_BRACKET;
    __keycodes[XK_apostrophe] = BVR_KEY_APOSTROPHE;
    __keycodes[XK_comma] = BVR_KEY_COMMA;
    __keycodes[XK_minus] = BVR_KEY_MINUS;
    __keycodes[XK_period] = BVR_KEY_PERIOD;
    __keycodes[XK_plus] = BVR_KEY_EQUAL;

    // system
    __keycodes[XK_space] = BVR_KEY_SPACE;
    __keycodes[XK_Menu] = BVR_KEY_MENU;
    __keycodes[XK_Escape] = BVR_KEY_ESCAPE;
    __keycodes[XK_Tab] = BVR_KEY_TAB;
    __keycodes[XK_BackSpace] = BVR_KEY_BACKSPACE;
    __keycodes[XK_Insert] = BVR_KEY_INSERT;
    __keycodes[XK_Delete] = BVR_KEY_DELETE;
    __keycodes[XK_Right] = BVR_KEY_RIGHT;
    __keycodes[XK_Left] = BVR_KEY_LEFT;
    __keycodes[XK_Down] = BVR_KEY_DOWN;
    __keycodes[XK_Up] = BVR_KEY_UP;
    __keycodes[XK_Page_Up] = BVR_KEY_PAGE_UP;
    __keycodes[XK_Page_Down] = BVR_KEY_PAGE_DOWN;
    __keycodes[XK_Home] = BVR_KEY_HOME;
    __keycodes[XK_End] = BVR_KEY_END;

    // system
    __keycodes[XK_Caps_Lock] = BVR_KEY_CAPS_LOCK;
    __keycodes[XK_Scroll_Lock] = BVR_KEY_SCROLL_LOCK;
    __keycodes[XK_Num_Lock] = BVR_KEY_NUM_LOCK;
    __keycodes[XK_Print] = BVR_KEY_PRINT_SCREEN;
    __keycodes[XK_Pause] = BVR_KEY_PAUSE;
    
    // function
    __keycodes[XK_F1] = BVR_KEY_F1;
    __keycodes[XK_F2] = BVR_KEY_F2;
    __keycodes[XK_F3] = BVR_KEY_F3;
    __keycodes[XK_F4] = BVR_KEY_F4;
    __keycodes[XK_F5] = BVR_KEY_F5;
    __keycodes[XK_F6] = BVR_KEY_F6;
    __keycodes[XK_F7] = BVR_KEY_F7;
    __keycodes[XK_F8] = BVR_KEY_F8;
    __keycodes[XK_F9] = BVR_KEY_F9;
    __keycodes[XK_F10] = BVR_KEY_F10;
    __keycodes[XK_F11] = BVR_KEY_F11;
    __keycodes[XK_F12] = BVR_KEY_F12;

    __keycodes[XK_KP_0] = BVR_KEY_KP_0;
    __keycodes[XK_KP_1] = BVR_KEY_KP_1;
    __keycodes[XK_KP_2] = BVR_KEY_KP_2;
    __keycodes[XK_KP_3] = BVR_KEY_KP_3;
    __keycodes[XK_KP_4] = BVR_KEY_KP_4;
    __keycodes[XK_KP_5] = BVR_KEY_KP_5;
    __keycodes[XK_KP_6] = BVR_KEY_KP_6;
    __keycodes[XK_KP_7] = BVR_KEY_KP_7;
    __keycodes[XK_KP_8] = BVR_KEY_KP_8;
    __keycodes[XK_KP_9] = BVR_KEY_KP_9;
    __keycodes[XK_KP_Decimal] = BVR_KEY_KP_DECIMAL;
    __keycodes[XK_KP_Divide] = BVR_KEY_KP_DIVIDE;
    __keycodes[XK_KP_Multiply] = BVR_KEY_KP_MULTIPLY;
    __keycodes[XK_KP_Subtract] = BVR_KEY_KP_SUBTRACT;
    __keycodes[XK_KP_Add] = BVR_KEY_KP_ADD;
    __keycodes[XK_KP_Enter] = BVR_KEY_KP_ENTER;
    __keycodes[XK_KP_Equal] = BVR_KEY_KP_EQUAL;

    __keycodes[XK_Control_L] = BVR_KEY_LEFT_CONTROL;
    __keycodes[XK_Control_R] = BVR_KEY_LEFT_CONTROL;
    __keycodes[XK_Shift_L] = BVR_KEY_LEFT_SHIFT;
    __keycodes[XK_Shift_R] = BVR_KEY_LEFT_SHIFT;
    __keycodes[XK_Alt_L] = BVR_KEY_LEFT_ALT;
    __keycodes[XK_Alt_R] = BVR_KEY_LEFT_ALT;
}

static void bvri_window_poll_events_impl(bvr_window_t* window){
    XEvent event;

    // reset scroll
    window->inputs.scroll = 0.0f;

    // poll events
    while(XPending(window->handle.x11.xdisplay)){
        XNextEvent(window->handle.x11.xdisplay, &event);

        switch (event.type)
        {
        case ClientMessage:
            // close event
            if (event.xclient.data.l[0] == window->handle.x11.atoms[BVRI_WM_ATOM_DELETE])
            {
                window->awake = 0;
                return;
            }
            break;

        case DestroyNotify:
            window->awake = 0;
            return;

        case FocusIn:
        case FocusOut:
            window->focus = !window->focus;
            break;

        case KeymapNotify:
            XRefreshKeyboardMapping(&event.xmapping);
            break;

        case KeyPress:
        case KeyRelease:
            {
                int action = event.type == KeyRelease ? BVR_INPUT_RELEASE : BVR_INPUT_PRESSED;
                int key = __keycodes[XLookupKeysym(&event.xkey, 0)];

                if (window->inputs.keys[key] == BVR_INPUT_PRESSED && action != BVR_INPUT_RELEASE)
                {
                    action = BVR_INPUT_DOWN;
                }

                window->inputs.keys[key] = action;
            }
            break;

        case ButtonPress:
        case ButtonRelease:
            {
                int action = event.type == ButtonRelease ? BVR_INPUT_RELEASE : BVR_INPUT_PRESSED;
                int button = event.xbutton.button;

                // button assigns to scroll
                if (button == 4 || button == 5)
                {
                    window->inputs.scroll = button == 5 ? -1.0f : 1.0f;
                }
                else
                {
                    if (window->inputs.buttons[button] == BVR_INPUT_PRESSED && action != BVR_INPUT_RELEASE)
                    {
                        action = BVR_INPUT_DOWN;
                    }

                    window->inputs.buttons[button] = action;
                }
            }
            break;

        case MotionNotify:
            {
                window->inputs.mouse[0] = event.xmotion.x;
                window->inputs.mouse[1] = event.xmotion.y;
                window->inputs.prev_mouse[0] = window->inputs.mouse[0];
                window->inputs.prev_mouse[1] = window->inputs.mouse[1];
                window->inputs.motion[0] = window->inputs.prev_mouse[0] - window->inputs.mouse[0];
                window->inputs.motion[0] = window->inputs.prev_mouse[1] - window->inputs.mouse[1];
                window->inputs.relative_motion[0] = window->inputs.motion[0];
                window->inputs.relative_motion[1] = window->inputs.motion[1];
            }
            break;

        case ResizeRequest:
            {
                if (window->awake && window->context)
                {
                    window->width = event.xresizerequest.width;
                    window->height = event.xresizerequest.height;

                    if (window->width > 0 && window->height > 0)
                    {
                        glViewport(0, 0, window->width, window->height);
                    }
                }
            }

        default:
            break;
        }
    }
    
}

static void bvri_window_push_buffers_impl(bvr_window_t* window){
    glXSwapBuffers(window->handle.x11.xdisplay, window->handle.x11.xwindow);
}

static void bvr_window_set_size_impl(bvr_window_t* window, const uint16 width, const uint16 height){
    XResizeWindow(
        window->handle.x11.xdisplay, 
        window->handle.x11.xwindow,
        width, height
    );
}

static void bvr_window_set_position_impl(bvr_window_t* window, const uint16 x, const uint16 y){
    XMoveWindow(
        window->handle.x11.xdisplay, 
        window->handle.x11.xwindow,
        x, y
    );
}

static void bvr_window_set_name_impl(bvr_window_t* window, const char* name){
    // change window name
    XChangeProperty(
        window->handle.x11.xdisplay,
        window->handle.x11.xwindow,
        window->handle.x11.atoms[BVRI_WM_ATOM_NAME],
        window->handle.x11.atoms[BVRI_WM_ATOM_UTF8],
        8, PropModeReplace, name, strlen(name)
    );

    // change window icon name
    XChangeProperty(
        window->handle.x11.xdisplay,
        window->handle.x11.xwindow,
        window->handle.x11.atoms[BVRI_WM_ATOM_ICON_NAME],
        window->handle.x11.atoms[BVRI_WM_ATOM_UTF8],
        8, PropModeReplace, name, strlen(name)
    );

    XFlush(window->handle.x11.xdisplay);
}

// window destroy
static void bvri_window_destroy_impl(bvr_window_t* window){
    if(window->context){
        glXDestroyContext(window->handle.x11.xdisplay, window->context);
    }

    XFree(window->handle.x11.xvisual);
    XFreeColormap(window->handle.x11.xdisplay, window->handle.x11.xcolormap);
    XDestroyWindow(window->handle.x11.xdisplay, window->handle.x11.xwindow);
    XCloseDisplay(window->handle.x11.xdisplay);
}

static void* bvri_load_proc(const char* name){
    return glXGetProcAddress(name);
}

/*
https://github.com/ThomasHabets/monotonic_clock/tree/master
*/
static uint64 bvri_get_ns_tick_impl(){
    uint64 tick;

    struct timespec now;
    struct timeval tv;

    if(clock_gettime(_POSIX_MONOTONIC_CLOCK, &now) == 0){
        tick = (uint64)now.tv_sec;

        // number of microsec in one second
        tick *= 1000000000UL;
        tick += (uint64)now.tv_nsec;
    }
    else if(gettimeofday(&tv, NULL) == 0) {
        // fallback 
        tick = (uint64)tv.tv_sec * 1000000UL + (uint64)tv.tv_usec;
    }
    else {
        BVR_ASSERT(0 && "invalid clock system");
    }

    return tick;
}

static void bvri_thread_wait_ns(uint64 ns){
    int has_err = 1;
    struct timespec now, remaining;
    remaining.tv_sec = (time_t)(ns / 1000000000UL);
    remaining.tv_nsec = (long)(ns % 1000000000UL);

    do
    {
        now.tv_sec = remaining.tv_sec;
        now.tv_nsec = remaining.tv_nsec;
        has_err = nanosleep(&now, &remaining);
    } while(has_err);
}

#else
    #error unsupported window system
#endif

int bvr_create_window(bvr_window_t* window, const uint16 width, const uint16 height, const char* title, const int flags){
    BVR_ASSERT(window);
    BVR_ASSERT(width > 0 && height > 0);

    int state = 1;

    __window = window;

    window->awake = 0;
    window->focus = 0;
    window->flags = flags;
    window->framebuffer.width = width;
    window->framebuffer.height = height;
    window->framebuffer.buffer = 0;

    state = bvri_create_window_impl(window, width, height, title, flags);
    bvri_create_keymap_layout();

    memset(window->inputs.keys, 0, BVR_KEYBOARD_SIZE * sizeof(char));
    memset(window->inputs.buttons, 0, BVR_MOUSE_SIZE * sizeof(char));
    window->inputs.sensivity = 1.0f;
    window->inputs.scroll = 0.0f;
    window->inputs.grab = 0;

    // init chrono
    window->timer.initial_frame = 0;
    window->timer.current_time = 0;
    window->timer.previous_time = 0;
    window->timer.delta_time = 0.0f;

    // create default keyaxis
    window->inputs.axis.horizontal.keys[0] = BVR_KEY_RIGHT;
    window->inputs.axis.horizontal.keys[1] = BVR_KEY_LEFT;
    window->inputs.axis.horizontal.alt_keys[0] = BVR_KEY_D;
    window->inputs.axis.horizontal.alt_keys[1] = BVR_KEY_A;

    window->inputs.axis.vertical.keys[0] = BVR_KEY_UP;
    window->inputs.axis.vertical.keys[1] = BVR_KEY_DOWN;
    window->inputs.axis.vertical.alt_keys[0] = BVR_KEY_W;
    window->inputs.axis.vertical.alt_keys[1] = BVR_KEY_S;

    // load opengl functions
    BVR_ASSERT(bvr_load_gl((GLADloadproc)bvri_load_proc));

    BVR_STRCPY(window->vendor.version, "Beauvoir " BVR_VERSION, 31);
    BVR_STRCPY(window->vendor.name, glGetString(GL_VENDOR), 31);
    BVR_STRCPY(window->vendor.gl_version, glGetString(GL_VERSION), 31);
    BVR_STRCPY(window->vendor.glsl_version, glGetString(GL_SHADING_LANGUAGE_VERSION), 31);

    BVR_PRINTF("Running OPENGL %s", window->vendor.gl_version);
    BVR_PRINTF("Running GLSL %s", window->vendor.glsl_version);

    // enable debugging
    if(glad_glDebugMessageCallback){
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(bvr_error_callback, NULL);
    }

    window->awake = 1;
    window->focus = 1;
    
    bvr_create_framebuffer(&window->framebuffer, width, height, NULL);

    return state;
}

void bvr_window_poll_events(bvr_window_t* window){
    bvri_window_poll_events_impl(window);

    window->timer.current_time = bvr_get_frame();
    window->timer.delta_time = window->timer.current_time - window->timer.previous_time;
}

void bvr_window_push_buffers(bvr_window_t* window){
    bvri_window_push_buffers_impl(window);

    window->timer.previous_time = window->timer.current_time;
}

void bvr_window_set_size(bvr_window_t* window, const uint16 width, const uint16 height){
    BVR_ASSERT(window);
    
    // when attempting to create a window with 
    // an impossible width or height
    if(width == 0 || height == 0 || !window->awake){
        return;
    }

    window->width = width;
    window->height = height;

    bvr_window_set_size_impl(window, width, height);

    glViewport(0, 0, width, height);
}

void bvr_window_set_position(bvr_window_t* window, const uint16 x, const uint16 y){
    BVR_ASSERT(window);

    window->x = x;
    window->y = y;

    bvr_window_set_position_impl(window, x, y);
}

void bvr_destroy_window(bvr_window_t* window){
    bvr_destroy_framebuffer(&window->framebuffer);
    bvri_window_destroy_impl(window);    
}

int bvr_key_down(uint16 key){
    return  __window->inputs.keys[key] == BVR_INPUT_DOWN || 
            __window->inputs.keys[key] == BVR_INPUT_PRESSED;
}

int bvr_key_presssed(uint16 key){
    return __window->inputs.keys[key] == BVR_INPUT_PRESSED;
}

int bvr_axis_down(bvr_keyaxis_t* axis){
    int positive = bvr_key_down(axis->keys[0]) || bvr_key_down(axis->alt_keys[0]);
    int negative = bvr_key_down(axis->keys[1]) || bvr_key_down(axis->alt_keys[1]);
    return positive - negative;
}

int bvr_axis_presssed(bvr_keyaxis_t* axis){
    int positive = bvr_key_presssed(axis->keys[0]) || bvr_key_presssed(axis->alt_keys[0]);
    int negative = bvr_key_presssed(axis->keys[1]) || bvr_key_presssed(axis->alt_keys[1]);
    return positive - negative;
}

int bvr_button_down(uint16 button){
    return __window->inputs.buttons[button] == BVR_INPUT_DOWN ||
        __window->inputs.buttons[button] == BVR_INPUT_PRESSED;
}

int bvr_button_pressed(uint16 button){
    return __window->inputs.buttons[button] == BVR_INPUT_PRESSED;
}

void bvr_mouse_position(float* x, float* y){
    *x = __window->inputs.mouse[0];
    *y = __window->inputs.mouse[1];
}

void bvr_mouse_relative_position(float* x, float *y){
    *x = __window->inputs.relative_motion[0];
    *y = __window->inputs.relative_motion[1];
}

float bvr_mouse_scroll(){
    return __window->inputs.scroll;
}

void bvri_file_dialog_callback(void (*userdata) (bvr_string_t* path), const char * const *filelist, int filter){
    BVR_ASSERT(0 && "not implemented");
}

void bvr_open_file_dialog(void (*callback) (bvr_string_t* path), const char* filters, bool allow_many){
    BVR_ASSERT(0 && "not implemented");
}
 
uint64 bvr_get_frame(){
    uint64 frame;
    
    if(__window->timer.initial_frame == 0){
        __window->timer.initial_frame = bvri_get_ns_tick_impl();
    }

    frame = bvri_get_ns_tick_impl() - __window->timer.initial_frame;
    return frame / 1000000UL;
}

float bvr_get_delta_time(void){
    return __window->timer.delta_time / 1000.0;
}

void bvr_delay(uint64 ms){
    bvri_thread_wait_ns(ms * 1000000UL);
}

void static bvr_error_callback(GLenum source, GLenum type, GLuint id,
   GLenum severity, GLsizei length, const GLchar* message, const void* userParam){
    
    if(severity == GL_DEBUG_SEVERITY_NOTIFICATION){
        return;
    }

    char src[25];
    char error[25];

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:
            BVR_STRCPY(src, "API", 4);
            break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            BVR_STRCPY(src, "WINDOW SYSTEM", 14);
            break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            BVR_STRCPY(src, "SHADERS", 8);
            break;
        
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            BVR_STRCPY(src, "THIRD PARTY", 12);
            break;
        
        case GL_DEBUG_SOURCE_APPLICATION:
            BVR_STRCPY(src, "APPLICATION", 12);
            break;
        
        case GL_DEBUG_SOURCE_OTHER:
            BVR_STRCPY(src, "OTHER", 6);
            break;
    };
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
            BVR_STRCPY(error, "fatal error", 12);
            BVR_PRINTF("catch a new %s (%i) from OGL %s! '%s'", error, id, src, message);
            BVR_ASSERT(0);
            break;

        // case GL_DEBUG_SEVERITY_MEDIUM:
        //     BVR_STRCPY(error, "medium error", 13);
        //     break;
        // 
        // case GL_DEBUG_SEVERITY_LOW:
        //     BVR_STRCPY(error, "warning", 8);
        //     break;
        // 
        // case GL_DEBUG_SEVERITY_NOTIFICATION:
        //     BVR_STRCPY(error, "notification", 13);
        //     break;
        
        default:
            //BVR_STRCPY(error, "unknown", 6);
            break;
        
    };

}