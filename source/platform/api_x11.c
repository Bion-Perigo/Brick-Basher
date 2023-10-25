#ifdef PLATFORM_LINUX
#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "X11/X.h"
#include "core.h"

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdbool.h>
#include <string.h>

/*==================== Extern ====================*/

extern char current_keys[KEY_MAX];
extern char previous_keys[KEY_MAX];

extern void platform_linux_set_window_info(EGLNativeDisplayType dpy, EGLNativeWindowType win);

/*==================== Variables ====================*/

static void *lib_x11 = NULL;

static const char *x11_names[] = {
    "XkbSetDetectableAutoRepeat", //
    "XInternAtom",                //
    "XSetWMProtocols",            //
    "XStoreName",                 //
    "XOpenDisplay",               //
    "XCloseDisplay",              //
    "XMapWindow",                 //
    "XUnmapWindow",               //
    "XSelectInput",               //
    "XNextEvent",                 //
    "XSendEvent",                 //
    "XPending",                   //
    "XLookupKeysym",              //
    "XDestroyWindow",             //
    "XCreateSimpleWindow",        //
    "\0"                          //
};

static struct x11_api {
  int (*XkbSetDetectableAutoRepeat)(Display *, int, int *);
  Atom (*XInternAtom)(Display *, const char *, int);
  Atom (*XSetWMProtocols)(Display *, Window, Atom *, int);
  int (*XStoreName)(Display *, Window, const char *);
  Display *(*XOpenDisplay)(const char *);
  int (*XCloseDisplay)(Display *);
  int (*XMapWindow)(Display *, Window);
  int (*XUnmapWindow)(Display *, Window);
  int (*XSelectInput)(Display *, Window, long);
  int (*XNextEvent)(Display *, XEvent *);
  int (*XSendEvent)(Display *, Window, int, long, XEvent *);
  int (*XPending)(Display *);
  KeySym (*XLookupKeysym)(XKeyEvent *, int);
  int (*XDestroyWindow)(Display *, Window);
  Window (*XCreateSimpleWindow)(
      Display *, Window, int, int, unsigned int, unsigned int, unsigned int, unsigned long, unsigned long);
} x11_api;

static struct window_p {
  int width;
  int height;
  const char *title;
  int screen;
  bool should_close;
  bool fullscreen;

  NativeDisplayType display;
  NativeWindowType window;

  Atom wm_protocols;
  Atom wm_delete_window;
} x11_window;

/*==================== Declaration ====================*/

bool api_x11_init(struct window_api_p *win_api);
struct window_p *api_x11_create_window(int width, int height, const char *title);
void api_x11_update_window();
void api_x11_close_window();
bool api_x11_window_should_close();
void api_x11_set_window_fullscreen();
void api_x11_get_window_size(int *width, int *height);
void api_x11_update_keybord(int key_code, bool is_pressed);

/*==================== Definition ====================*/

bool api_x11_init(struct window_api_p *win_api) {
  lib_x11 = load_library_p("libX11.so");
  if (lib_x11 == NULL) {
    return false;
  }

  get_functions_p(lib_x11, &x11_api, x11_names);
  x11_window.should_close = true;

  win_api->on_create_window = &api_x11_create_window;
  win_api->on_update_window = &api_x11_update_window;
  win_api->on_close_window = &api_x11_close_window;
  win_api->on_window_should_close = &api_x11_window_should_close;
  win_api->on_set_window_fullscreen = &api_x11_set_window_fullscreen;
  win_api->on_get_window_size = &api_x11_get_window_size;

  return true;
}

struct window_p *api_x11_create_window(int width, int height, const char *title) {
  if (x11_window.should_close == false) {
    return &x11_window;
  }

  long mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask;

  x11_window.display = x11_api.XOpenDisplay(NULL);
  x11_window.screen = DefaultScreen(x11_window.display);
  x11_window.window = x11_api.XCreateSimpleWindow(
      x11_window.display, DefaultRootWindow(x11_window.display), 0, 0, width, height, 1,
      BlackPixel(x11_window.display, x11_window.screen), WhitePixel(x11_window.display, x11_window.screen));

  x11_api.XMapWindow(x11_window.display, x11_window.window);
  x11_api.XSelectInput(x11_window.display, x11_window.window, mask);
  x11_api.XStoreName(x11_window.display, x11_window.window, title);
  x11_api.XkbSetDetectableAutoRepeat(x11_window.display, true, NULL);

  x11_window.wm_protocols = x11_api.XInternAtom(x11_window.display, "WM_PROTOCOLS", false);
  x11_window.wm_delete_window = x11_api.XInternAtom(x11_window.display, "WM_DELETE_WINDOW", false);

  x11_api.XSetWMProtocols(x11_window.display, x11_window.window, &x11_window.wm_delete_window, true);

  x11_window.width = width;
  x11_window.height = height;
  x11_window.title = title;
  x11_window.should_close = false;
  G_LOG(LOG_INFO, "Init Window: Width:%d Height:%d Title:%s", width, height, title);
  platform_linux_set_window_info(x11_window.display, x11_window.window);

  return &x11_window;
}

void api_x11_update_window() {
  XEvent event;
  copy_memory_f(previous_keys, current_keys, sizeof(char) * KEY_MAX);

  while (x11_api.XPending(x11_window.display)) {
    x11_api.XNextEvent(x11_window.display, &event);

    switch (event.type) {
    case KeyPress: {
      int key_code = x11_api.XLookupKeysym(&event.xkey, 1);
      api_x11_update_keybord(key_code, true);
    } break;
    case KeyRelease: {
      int key_code = x11_api.XLookupKeysym(&event.xkey, 1);
      api_x11_update_keybord(key_code, false);
    } break;
    case ButtonPress: {
      G_LOG(LOG_SUCCESS, "ButtonRelease:%d", event.xkey.keycode);
    } break;
    case ButtonRelease: {
      G_LOG(LOG_WARNING, "ButtonRelease:%d", event.xbutton.button);
    } break;
    case ConfigureNotify: {
      x11_window.width = event.xconfigurerequest.width;
      x11_window.height = event.xconfigurerequest.height;
      G_LOG(LOG_WARNING, "Window Size:%d x %d", x11_window.width, x11_window.height);
    } break;
    case ClientMessage: {
      if (event.xclient.message_type == x11_window.wm_protocols) {
        if (event.xclient.data.l[0] == (long)x11_window.wm_delete_window) {
          x11_window.should_close = true;
        }
      }
    } break;
    }
  }
}

void api_x11_close_window() {
  x11_api.XUnmapWindow(x11_window.display, x11_window.window);
  x11_api.XDestroyWindow(x11_window.display, x11_window.window);
  x11_api.XCloseDisplay(x11_window.display);
}

bool api_x11_window_should_close() {
  return x11_window.should_close;
}

void api_x11_set_window_fullscreen() {
  XEvent event;
  x11_window.fullscreen = !x11_window.fullscreen;
  Atom wm_state = x11_api.XInternAtom(x11_window.display, "_NET_WM_STATE", false);
  Atom wm_fullscreen = x11_api.XInternAtom(x11_window.display, "_NET_WM_STATE_FULLSCREEN", false);
  memset(&event, 0, 1);
  event.type = ClientMessage;
  event.xclient.window = x11_window.window;
  event.xclient.message_type = wm_state;
  event.xclient.format = 32;
  event.xclient.data.l[0] = x11_window.fullscreen;
  event.xclient.data.l[1] = wm_fullscreen;
  event.xclient.data.l[2] = 0;
  x11_api.XSendEvent(
      x11_window.display, DefaultRootWindow(x11_window.display), false, StructureNotifyMask, &event);
}

void api_x11_get_window_size(int *width, int *height) {
  *width = x11_window.width;
  *height = x11_window.height;
}

void api_x11_update_keybord(int key_code, bool is_pressed) {
#define CHECK_KEY(game_key, key_code, is_pressed) \
  if (api_X11_keys[game_key] == key_code) {       \
    current_keys[game_key] = is_pressed;          \
    return;                                       \
  }

  static int api_X11_keys[] = {
      XK_apostrophe,   // Key: '
      XK_comma,        // Key: ,
      XK_minus,        // Key: -
      XK_period,       // Key: .
      XK_slash,        // Key: /
      XK_0,            // Key: 0
      XK_1,            // Key: 1
      XK_2,            // Key: 2
      XK_3,            // Key: 3
      XK_4,            // Key: 4
      XK_5,            // Key: 5
      XK_6,            // Key: 6
      XK_7,            // Key: 7
      XK_8,            // Key: 8
      XK_9,            // Key: 9
      XK_semicolon,    // Key: ;
      XK_equal,        // Key: =
      XK_A,            // Key: A
      XK_B,            // Key: B
      XK_C,            // Key: C
      XK_D,            // Key: D
      XK_E,            // Key: E
      XK_F,            // Key: F
      XK_G,            // Key: G
      XK_H,            // Key: H
      XK_I,            // Key: I
      XK_J,            // Key: J
      XK_K,            // Key: K
      XK_L,            // Key: L
      XK_M,            // Key: M
      XK_N,            // Key: N
      XK_O,            // Key: O
      XK_P,            // Key: P
      XK_Q,            // Key: Q
      XK_R,            // Key: R
      XK_S,            // Key: S
      XK_T,            // Key: T
      XK_U,            // Key: U
      XK_V,            // Key: V
      XK_W,            // Key: W
      XK_X,            // Key: X
      XK_Y,            // Key: Y
      XK_Z,            // Key: Z
      XK_bracketleft,  // Key: [{
      XK_backslash,    // Key: '\'
      XK_bracketright, // Key: ]}
      XK_grave,        // Key: `
      XK_space,        // Key: Space
      XK_Escape,       // Key: Escape
      XK_Return,       // Key: Enter
      XK_ISO_Left_Tab, // Key: Tab
      XK_BackSpace,    // Key: Backspace
      XK_Insert,       // Key: Insert
      XK_Delete,       // Key: Delete
      XK_Right,        // Key: Right
      XK_Left,         // Key: Left
      XK_Down,         // Key: Down
      XK_Up,           // Key: Up
      XK_Page_Down,    // Key: Page Down
      XK_Page_Up,      // Key: Page Up
      XK_Home,         // Key: Home
      XK_End,          // Key: End
      XK_Caps_Lock,    // Key: Caps Lock
      XK_Scroll_Lock,  // Key: Scroll Lock
      XK_Num_Lock,     // Key: Num Lock
      XK_Print,        // Key: Print
      XK_Pause,        // Key: Pause
      XK_F1,           // Key: F1
      XK_F2,           // Key: F2
      XK_F3,           // Key: F3
      XK_F4,           // Key: F4
      XK_F5,           // Key: F5
      XK_F6,           // Key: F6
      XK_F7,           // Key: F7
      XK_F8,           // Key: F8
      XK_F9,           // Key: F9
      XK_F10,          // Key: F10
      XK_F11,          // Key: F11
      XK_F12,          // Key: F12
      XK_Shift_L,      // Key: Left Shift
      XK_Control_L,    // Key: Left Control
      XK_Alt_L,        // Key: Left Alt
      XK_Super_L,      // Key: Left Super
      XK_Shift_R,      // Key: Right Shift
      XK_Control_R,    // Key: Right Control
      XK_Alt_R,        // Key: Right Alt
      XK_Super_R,      // Key: Right Super
      XK_KP_0,         // Key: 0
      XK_KP_1,         // Key: 1
      XK_KP_2,         // Key: 2
      XK_KP_3,         // Key: 3
      XK_KP_4,         // Key: 4
      XK_KP_5,         // Key: 5
      XK_KP_6,         // Key: 6
      XK_KP_7,         // Key: 7
      XK_KP_8,         // Key: 8
      XK_KP_9,         // Key: 9
      XK_KP_Decimal,   // Key: Decimal
      XK_KP_Divide,    // Key: Divide
      XK_KP_Multiply,  // Key: Multiply
      XK_KP_Subtract,  // Key: Subtract
      XK_KP_Add,       // Key: Add
      XK_KP_Enter,     // Key: Enter
      XK_KP_Equal      // Key: Equal
  };

  CHECK_KEY(KEY_APOSTROPHE, key_code, is_pressed)
  CHECK_KEY(KEY_COMMA, key_code, is_pressed)
  CHECK_KEY(KEY_MINUS, key_code, is_pressed)
  CHECK_KEY(KEY_PERIOD, key_code, is_pressed)
  CHECK_KEY(KEY_SLASH, key_code, is_pressed)
  CHECK_KEY(KEY_ZERO, key_code, is_pressed)
  CHECK_KEY(KEY_ONE, key_code, is_pressed)
  CHECK_KEY(KEY_TWO, key_code, is_pressed)
  CHECK_KEY(KEY_THREE, key_code, is_pressed)
  CHECK_KEY(KEY_FOUR, key_code, is_pressed)
  CHECK_KEY(KEY_FIVE, key_code, is_pressed)
  CHECK_KEY(KEY_SIX, key_code, is_pressed)
  CHECK_KEY(KEY_SEVEN, key_code, is_pressed)
  CHECK_KEY(KEY_EIGHT, key_code, is_pressed)
  CHECK_KEY(KEY_NINE, key_code, is_pressed)
  CHECK_KEY(KEY_SEMICOLON, key_code, is_pressed)
  CHECK_KEY(KEY_EQUAL, key_code, is_pressed)
  CHECK_KEY(KEY_A, key_code, is_pressed)
  CHECK_KEY(KEY_B, key_code, is_pressed)
  CHECK_KEY(KEY_C, key_code, is_pressed)
  CHECK_KEY(KEY_D, key_code, is_pressed)
  CHECK_KEY(KEY_E, key_code, is_pressed)
  CHECK_KEY(KEY_F, key_code, is_pressed)
  CHECK_KEY(KEY_G, key_code, is_pressed)
  CHECK_KEY(KEY_H, key_code, is_pressed)
  CHECK_KEY(KEY_I, key_code, is_pressed)
  CHECK_KEY(KEY_J, key_code, is_pressed)
  CHECK_KEY(KEY_K, key_code, is_pressed)
  CHECK_KEY(KEY_L, key_code, is_pressed)
  CHECK_KEY(KEY_M, key_code, is_pressed)
  CHECK_KEY(KEY_N, key_code, is_pressed)
  CHECK_KEY(KEY_O, key_code, is_pressed)
  CHECK_KEY(KEY_P, key_code, is_pressed)
  CHECK_KEY(KEY_Q, key_code, is_pressed)
  CHECK_KEY(KEY_R, key_code, is_pressed)
  CHECK_KEY(KEY_S, key_code, is_pressed)
  CHECK_KEY(KEY_T, key_code, is_pressed)
  CHECK_KEY(KEY_U, key_code, is_pressed)
  CHECK_KEY(KEY_V, key_code, is_pressed)
  CHECK_KEY(KEY_W, key_code, is_pressed)
  CHECK_KEY(KEY_X, key_code, is_pressed)
  CHECK_KEY(KEY_Y, key_code, is_pressed)
  CHECK_KEY(KEY_Z, key_code, is_pressed)
  CHECK_KEY(KEY_LEFT_BRACKET, key_code, is_pressed)
  CHECK_KEY(KEY_BACKSLASH, key_code, is_pressed)
  CHECK_KEY(KEY_RIGHT_BRACKET, key_code, is_pressed)
  CHECK_KEY(KEY_GRAVE, key_code, is_pressed)
  CHECK_KEY(KEY_SPACE, key_code, is_pressed)
  CHECK_KEY(KEY_ENTER, key_code, is_pressed)
  CHECK_KEY(KEY_ESCAPE, key_code, is_pressed)
  CHECK_KEY(KEY_TAB, key_code, is_pressed)
  CHECK_KEY(KEY_BACKSPACE, key_code, is_pressed)
  CHECK_KEY(KEY_INSERT, key_code, is_pressed)
  CHECK_KEY(KEY_DELETE, key_code, is_pressed)
  CHECK_KEY(KEY_RIGHT, key_code, is_pressed)
  CHECK_KEY(KEY_LEFT, key_code, is_pressed)
  CHECK_KEY(KEY_DOWN, key_code, is_pressed)
  CHECK_KEY(KEY_UP, key_code, is_pressed)
  CHECK_KEY(KEY_PAGE_UP, key_code, is_pressed)
  CHECK_KEY(KEY_PAGE_DOWN, key_code, is_pressed)
  CHECK_KEY(KEY_HOME, key_code, is_pressed)
  CHECK_KEY(KEY_END, key_code, is_pressed)
  CHECK_KEY(KEY_CAPS_LOCK, key_code, is_pressed)
  CHECK_KEY(KEY_SCROLL_LOCK, key_code, is_pressed)
  CHECK_KEY(KEY_NUM_LOCK, key_code, is_pressed)
  CHECK_KEY(KEY_PRINT_SCREEN, key_code, is_pressed)
  CHECK_KEY(KEY_PAUSE, key_code, is_pressed)
  CHECK_KEY(KEY_F1, key_code, is_pressed)
  CHECK_KEY(KEY_F2, key_code, is_pressed)
  CHECK_KEY(KEY_F3, key_code, is_pressed)
  CHECK_KEY(KEY_F4, key_code, is_pressed)
  CHECK_KEY(KEY_F5, key_code, is_pressed)
  CHECK_KEY(KEY_F6, key_code, is_pressed)
  CHECK_KEY(KEY_F7, key_code, is_pressed)
  CHECK_KEY(KEY_F8, key_code, is_pressed)
  CHECK_KEY(KEY_F9, key_code, is_pressed)
  CHECK_KEY(KEY_F10, key_code, is_pressed)
  CHECK_KEY(KEY_F11, key_code, is_pressed)
  CHECK_KEY(KEY_F12, key_code, is_pressed)
  CHECK_KEY(KEY_LEFT_SHIFT, key_code, is_pressed)
  CHECK_KEY(KEY_LEFT_CONTROL, key_code, is_pressed)
  CHECK_KEY(KEY_LEFT_ALT, key_code, is_pressed)
  CHECK_KEY(KEY_LEFT_SUPER, key_code, is_pressed)
  CHECK_KEY(KEY_RIGHT_SHIFT, key_code, is_pressed)
  CHECK_KEY(KEY_RIGHT_CONTROL, key_code, is_pressed)
  CHECK_KEY(KEY_RIGHT_ALT, key_code, is_pressed)
  CHECK_KEY(KEY_RIGHT_SUPER, key_code, is_pressed)
  CHECK_KEY(KEY_KB_MENU, key_code, is_pressed)
  CHECK_KEY(KEY_KP_0, key_code, is_pressed)
  CHECK_KEY(KEY_KP_1, key_code, is_pressed)
  CHECK_KEY(KEY_KP_2, key_code, is_pressed)
  CHECK_KEY(KEY_KP_3, key_code, is_pressed)
  CHECK_KEY(KEY_KP_4, key_code, is_pressed)
  CHECK_KEY(KEY_KP_5, key_code, is_pressed)
  CHECK_KEY(KEY_KP_6, key_code, is_pressed)
  CHECK_KEY(KEY_KP_7, key_code, is_pressed)
  CHECK_KEY(KEY_KP_8, key_code, is_pressed)
  CHECK_KEY(KEY_KP_9, key_code, is_pressed)
  CHECK_KEY(KEY_KP_DECIMAL, key_code, is_pressed)
  CHECK_KEY(KEY_KP_DIVIDE, key_code, is_pressed)
  CHECK_KEY(KEY_KP_MULTIPLY, key_code, is_pressed)
  CHECK_KEY(KEY_KP_SUBTRACT, key_code, is_pressed)
  CHECK_KEY(KEY_KP_ADD, key_code, is_pressed)
  CHECK_KEY(KEY_KP_ENTER, key_code, is_pressed)
}

#endif // PLATFORM_LINUX
