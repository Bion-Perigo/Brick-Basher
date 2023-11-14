#ifdef PLATFORM_WINDOWS
#include "EGL/eglplatform.h"

#include <core.h>
#include <libloaderapi.h>
#include <stdbool.h>
#include <stdio.h>
#include <windef.h>
#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>
#include <winnt.h>
#include <winuser.h>

/*====================   ====================*/

#define CODE_WINDOW_CLOSE WM_USER + 1

/*==================== Extern ====================*/

extern char current_keys[KEY_MAX];
extern char previous_keys[KEY_MAX];
extern int mouse_position[2];

/*==================== APIs ====================*/

void *lib_user = NULL;

static const char *lib_user_names[] = {
    "SetWindowTextA",    //
    "ShowCursor",        //
    "RegisterClassExA",  //
    "DefWindowProcA",    //
    "LoadIconA",         //
    "LoadCursorA",       //
    "PeekMessageA",      //
    "TranslateMessage",  //
    "DispatchMessageA",  //
    "PostMessageA",      //
    "GetSystemMetrics",  //
    "SetWindowLongPtrA", //
    "SetWindowPos",      //
    "GetWindowRect",     //
    "GetClientRect",     //
    "AdjustWindowRect",  //
    "DestroyWindow",     //
    "GetClassInfoExA",   //
    "GetDC",             //
    "CreateWindowExA",   //
    "\0"                 //
};

static struct api_user {
  BOOL (*SetWindowTextA)(HWND hWnd, LPCSTR lpString);
  int (*ShowCursor)(BOOL bShow);
  ATOM (*RegisterClassExA)(const WNDCLASSEXA *unnamedParam1);
  LRESULT (*DefWindowProcA)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
  HICON (*LoadIconA)(HINSTANCE hInstance, LPCSTR lpIconName);
  HCURSOR (*LoadCursorA)(HINSTANCE hInstance, LPCSTR lpCursorName);
  BOOL (*PeekMessageA)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
  BOOL (*TranslateMessage)(const MSG *lpMsg);
  LRESULT (*DispatchMessageA)(const MSG *lpMsg);
  BOOL (*PostMessageA)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
  int (*GetSystemMetrics)(int nIndex);
  LONG_PTR (*SetWindowLongPtrA)(HWND hWnd, int nIndex, LONG_PTR dwNewLong);
  BOOL (*SetWindowPos)(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
  BOOL (*GetWindowRect)(HWND hWnd, LPRECT lpRect);
  BOOL (*GetClientRect)(HWND hWnd, LPRECT lpRect);
  BOOL (*AdjustWindowRect)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu);
  BOOL (*DestroyWindow)(HWND hWnd);
  BOOL (*GetClassInfoExA)(HINSTANCE hInstance, LPCSTR lpszClass, LPWNDCLASSEXA lpwcx);
  HDC (*GetDC)(HWND hWnd);
  HWND(*CreateWindowExA)
  (DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
   HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
} api_user;

/*==================== Variables ====================*/

static struct win32_window {
  struct window_p main_window;
  int last_width;
  int last_height;
  int last_pos_x;
  int last_pos_y;
} win32_window;

/*==================== Declaration ====================*/

bool api_win_init(struct window_api_p *win_api);
struct window_p *api_win32_create_window(int width, int height, const char *title);
LRESULT api_win32_win_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
void api_win32_update_window();
void api_win32_close_window();
void api_win32_set_window_fullscreen();
void api_win32_set_show_cursor(bool b_show);
void api_win32_set_window_title_info();
struct window_p *api_win32_create_dummy_window();
void api_win32_close_dummy_window(struct window_p *dummy_window);
void api_win32_update_keybord(int key_code, bool is_pressed);

/*==================== Definition  ====================*/

extern bool api_win_init(struct window_api_p *win_api) {
  lib_user = load_library_p("user32.dll");
  if (lib_user == NULL) {
    G_LOG(LOG_FATAL, "Librarie Not Loaded: USER32");
    return false;
  }

  get_functions_p(lib_user, &api_user, lib_user_names);

  win_api->on_create_window = &api_win32_create_window;
  win_api->on_update_window = &api_win32_update_window;
  win_api->on_close_window = &api_win32_close_window;
  win_api->on_set_window_fullscreen = &api_win32_set_window_fullscreen;
  win_api->on_show_cursor = &api_win32_set_show_cursor;

  return true;
}

struct window_p *api_win32_create_window(int width, int height, const char *title) {
  const char *window_class_name = "game_window";
  HINSTANCE h_instance = GetModuleHandle(NULL);

  WNDCLASSEXA wc = {0};

  if (!api_user.GetClassInfoExA(h_instance, window_class_name, &wc)) {
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = &api_win32_win_proc;
    wc.hInstance = h_instance;
    wc.hIcon = api_user.LoadIconA(NULL, IDI_APPLICATION);
    wc.hCursor = api_user.LoadCursorA(NULL, IDC_ARROW);
    wc.lpszClassName = window_class_name;

    if (!api_user.RegisterClassExA(&wc)) {
      G_LOG(LOG_FATAL, "Not Register Class Window");
      return NULL;
    }
  }

  RECT win_rect = {(LONG)0, (LONG)0, (LONG)width, (LONG)height};

  if (!api_user.AdjustWindowRect(&win_rect, WS_OVERLAPPEDWINDOW, FALSE)) {
    G_LOG(LOG_ERROR, "No Adjust Window Rect");
  }

  int win_width = win_rect.right - win_rect.left;
  int win_height = win_rect.bottom - win_rect.top;

  HWND window_handle = api_user.CreateWindowExA(
      0, window_class_name, title, WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
      win_width, win_height, NULL, NULL, h_instance, NULL);

  if (window_handle == NULL) {
    G_LOG(LOG_FATAL, "Not Create Game Window");
    return NULL;
  }

  struct window_p *win = &win32_window.main_window;
  RECT rect;
  api_user.GetClientRect(window_handle, &rect);
  win->width = rect.right - rect.left;
  win->height = rect.bottom - rect.top;
  win->title = title;
  win->fullscreen = false;
  win->should_close = false;
  win->window = window_handle;
  win->display = api_user.GetDC(window_handle);

  G_LOG(LOG_INFO, "Init Window => Width:%d Height:%d Title:%s", width, height, title);

  // Necessary so that the cursor can be changed after creating the window.
  set_show_cursor_p(false);
  set_show_cursor_p(true);

  return win;
}

LRESULT api_win32_win_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
  // struct window_p *win = &win32_window.main_window;

  switch (Msg) {
  case WM_CLOSE:
    api_user.PostMessageA(hWnd, CODE_WINDOW_CLOSE, 0, 0);
    break;

  case WM_KEYDOWN: {
    api_win32_update_keybord((int)wParam, true);
  } break;
  case WM_KEYUP: {
    api_win32_update_keybord((int)wParam, false);
  } break;

  case WM_MOUSEMOVE: {
    int mouse_x = GET_X_LPARAM(lParam);
    int mouse_y = GET_Y_LPARAM(lParam);
    mouse_position[0] = mouse_x; //(mouse_x > win->width) ? win->width : mouse_x;
    mouse_position[1] = mouse_y; //(mouse_y > win->height) ? win->height : mouse_y;
  } break;

  case WM_LBUTTONDOWN: {
    api_win32_update_keybord((int)MOUSE_LEFT_CODE, true);
  } break;
  case WM_LBUTTONUP: {
    api_win32_update_keybord((int)MOUSE_LEFT_CODE, false);
  } break;

  case WM_MBUTTONDOWN: {
    api_win32_update_keybord((int)MOUSE_MIDDLE_CODE, true);
  } break;
  case WM_MBUTTONUP: {
    api_win32_update_keybord((int)MOUSE_MIDDLE_CODE, false);
  } break;

  case WM_RBUTTONDOWN: {
    api_win32_update_keybord((int)MOUSE_RIGHT_CODE, true);
  } break;
  case WM_RBUTTONUP: {
    api_win32_update_keybord((int)MOUSE_RIGHT_CODE, false);
  } break;

  case WM_MOUSEWHEEL: {
    int delta = GET_WHEEL_DELTA_WPARAM(wParam);
    int key = (delta > 0) ? MOUSE_FORWARD_CODE : MOUSE_BACKWARD_CODE;
    api_win32_update_keybord(key, true);
  } break;

  case WM_SIZE: {
    if (!win32_window.main_window.fullscreen) {
      win32_window.main_window.width = LOWORD(lParam);
      win32_window.main_window.height = HIWORD(lParam);
    }
  } break;
  default:
    return api_user.DefWindowProcA(hWnd, Msg, wParam, lParam);
  }
  return 0;
}

void api_win32_update_window() {
#ifdef DEBUG_MODE
  api_win32_set_window_title_info();
#endif // DEBUG_MODE

  struct window_p *win = &win32_window.main_window;
  MSG msg;

  api_win32_set_window_title_info();
  copy_memory_f((char *)previous_keys, (char *)current_keys, sizeof(char) * KEY_MAX);
  api_win32_update_keybord(MOUSE_FORWARD_CODE, false);
  api_win32_update_keybord(MOUSE_BACKWARD_CODE, false);

  while (api_user.PeekMessageA(&msg, win->window, 0, 0, PM_REMOVE)) {
    if (msg.message == CODE_WINDOW_CLOSE) {
      win->should_close = true;
      return;
    }
    api_user.TranslateMessage(&msg);
    api_user.DispatchMessageA(&msg);
  }
}

void api_win32_close_window() {
  G_LOG(LOG_INFO, "Close Game Window");
  api_user.DestroyWindow(win32_window.main_window.window);
}

void api_win32_set_window_fullscreen() {
  struct window_p *win = &win32_window.main_window;
  win->fullscreen = !win->fullscreen;

  HWND window = win->window;
  if (win->fullscreen) {
    RECT rect;
    api_user.GetWindowRect(window, &rect);
    win->width = api_user.GetSystemMetrics(SM_CXSCREEN);
    win->height = api_user.GetSystemMetrics(SM_CYSCREEN);
    win32_window.last_pos_x = rect.left;
    win32_window.last_pos_y = rect.top;
    win32_window.last_width = rect.right - rect.left;
    win32_window.last_height = rect.bottom - rect.top;
    api_user.SetWindowLongPtrA(window, GWL_STYLE, WS_POPUP);
    api_user.SetWindowPos(window, NULL, 0, 0, win->width, win->height, SWP_SHOWWINDOW);
  } else {
    win->width = win32_window.last_width;
    win->height = win32_window.last_height;
    api_user.SetWindowLongPtrA(window, GWL_STYLE, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE);
    api_user.SetWindowPos(
        window, NULL, win32_window.last_pos_x, win32_window.last_pos_y, win->width, win->height, SWP_SHOWWINDOW);
  }
}

void api_win32_set_show_cursor(bool b_show) {
  api_user.ShowCursor(b_show);
}

void api_win32_set_window_title_info() {
  struct window_p *win = &win32_window.main_window;
  char buffer[100] = {""};
  int fps = get_fps_f();
  float ms = get_frametime_f();
  sprintf(buffer, "%s (Debug Mode) => FPS:%d | Ms:%f", win->title, fps, ms);
  api_user.SetWindowTextA(win->window, buffer);
}

struct window_p *api_win32_create_dummy_window();
struct window_p *api_win32_create_dummy_window() {
  const char *window_class_name = "dummy_window";
  HINSTANCE h_instance = GetModuleHandle(NULL);

  WNDCLASSEXA wc = {0};

  if (!api_user.GetClassInfoExA(h_instance, window_class_name, &wc)) {
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = &api_win32_win_proc;
    wc.hInstance = h_instance;
    wc.hIcon = api_user.LoadIconA(NULL, IDI_APPLICATION);
    wc.hCursor = api_user.LoadCursorA(NULL, IDC_ARROW);
    wc.lpszClassName = window_class_name;

    if (!api_user.RegisterClassExA(&wc)) {
      G_LOG(LOG_FATAL, "Not Register Class Window");
      return NULL;
    }
  }

  // Use the same size as main_window to prevent winproc from changing its size. \_(´-´)_/
  int x = win32_window.main_window.width;
  int y = win32_window.main_window.height;

  HWND window_handle = api_user.CreateWindowExA(
      0, window_class_name, "Dummy Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, x, y, NULL, NULL,
      h_instance, NULL);

  if (window_handle == NULL) {
    G_LOG(LOG_FATAL, "Not Create Dummy Window");
    return NULL;
  }

  struct window_p *win = (struct window_p *)get_memory_f(sizeof(struct window_p));
  win->width = x;
  win->height = y;
  win->title = NULL;
  win->fullscreen = false;
  win->should_close = false;
  win->window = window_handle;
  win->display = api_user.GetDC(window_handle);
  G_LOG(LOG_INFO, "Created Dummy Window");

  return win;
}

void api_win32_close_dummy_window(struct window_p *dummy_window) {
  api_user.DestroyWindow(dummy_window->window);
  free_memory_f((char *)dummy_window);
  G_LOG(LOG_INFO, "Close Dummy Window");
}

void api_win32_update_keybord(int key_code, bool is_pressed) {
#define CHECK_KEY(game_key, key_code, is_pressed) \
  if (api_win32_keys[game_key] == key_code) {     \
    current_keys[game_key] = is_pressed;          \
    return;                                       \
  }

  static int api_win32_keys[] = {
      VK_OEM_7,           // Key: '
      VK_OEM_COMMA,       // Key: ,
      VK_OEM_MINUS,       // Key: -
      VK_OEM_PERIOD,      // Key: .
      VK_OEM_2,           // Key: /
      0x30,               // Key: 0
      0x31,               // Key: 1
      0x32,               // Key: 2
      0x33,               // Key: 3
      0x34,               // Key: 4
      0x35,               // Key: 5
      0x36,               // Key: 6
      0x37,               // Key: 7
      0x38,               // Key: 8
      0x39,               // Key: 9
      VK_OEM_1,           // Key: ;
      VK_OEM_NEC_EQUAL,   // Key: =
      0x41,               // Key: A
      0x42,               // Key: B
      0x43,               // Key: C
      0x44,               // Key: D
      0x45,               // Key: E
      0x46,               // Key: F
      0x47,               // Key: G
      0x48,               // Key: H
      0x49,               // Key: I
      0x4A,               // Key: J
      0x4B,               // Key: K
      0x4C,               // Key: L
      0x4D,               // Key: M
      0x4E,               // Key: N
      0x4F,               // Key: O
      0x50,               // Key: P
      0x51,               // Key: Q
      0x52,               // Key: R
      0x53,               // Key: S
      0x54,               // Key: T
      0x55,               // Key: U
      0x56,               // Key: V
      0x57,               // Key: W
      0x58,               // Key: X
      0x59,               // Key: Y
      0x5A,               // Key: Z
      VK_OEM_4,           // Key: [{
      VK_OEM_102,         // Key: '\'
      VK_OEM_6,           // Key: ]}
      VK_OEM_3,           // Key: `
      VK_SPACE,           // Key: Space
      VK_ESCAPE,          // Key: Escape
      VK_RETURN,          // Key: Enter
      VK_TAB,             // Key: Tab
      VK_BACK,            // Key: Backspace
      VK_INSERT,          // Key: Insert
      VK_DELETE,          // Key: Delete
      VK_RIGHT,           // Key: Right
      VK_LEFT,            // Key: Left
      VK_DOWN,            // Key: Down
      VK_UP,              // Key: Up
      VK_NEXT,            // Key: Page Down
      VK_PRIOR,           // Key: Page Up
      VK_HOME,            // Key: Home
      VK_END,             // Key: End
      VK_CAPITAL,         // Key: Caps Look
      VK_SCROLL,          // Key: Scroll Down
      VK_NUMLOCK,         // Key: Num Lock
      VK_PRINT,           // Key: Print
      VK_PAUSE,           // Key: Pause
      VK_F1,              // Key: F1
      VK_F2,              // Key: F2
      VK_F3,              // Key: F3
      VK_F4,              // Key: F4
      VK_F5,              // Key: F5
      VK_F6,              // Key: F6
      VK_F7,              // Key: F7
      VK_F8,              // Key: F8
      VK_F9,              // Key: F9
      VK_F10,             // Key: F10
      VK_F11,             // Key: F11
      VK_F12,             // Key: F12
      VK_SHIFT,           // Key: Left Shift
      VK_CONTROL,         // Key: Left Control
      VK_MENU,            // Key: Left Alt
      VK_LWIN,            // Key: Left Super
      VK_SHIFT,           // Key: Left Shift
      VK_CONTROL,         // Key: Right Control
      VK_MENU,            // Key: Right Alt
      VK_RWIN,            // Key: Right Super
      VK_MENU,            // Key: Menu
      0x30,               // Key: 0
      0x31,               // Key: 1
      0x32,               // Key: 2
      0x33,               // Key: 3
      0x34,               // Key: 4
      0x35,               // Key: 5
      0x36,               // Key: 6
      0x37,               // Key: 7
      0x38,               // Key: 8
      0x39,               // Key: 9
      VK_DECIMAL,         // Key: de decimal
      VK_DIVIDE,          // Key: de divisão
      VK_MULTIPLY,        // Key: de multiplicação
      VK_SUBTRACT,        // Key: de subtração
      VK_ADD,             // Key: de adição
      VK_RETURN,          // Key: Enter
      VK_OEM_NEC_EQUAL,   // Key: Equal
      MOUSE_LEFT_CODE,    // Mouse Left
      MOUSE_MIDDLE_CODE,  // Mouse Middle
      MOUSE_RIGHT_CODE,   // Mouse Right
      MOUSE_FORWARD_CODE, // Mouse Forward
      MOUSE_BACKWARD_CODE // Mouse Backward
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
  CHECK_KEY(KEY_KP_EQUAL, key_code, is_pressed)
  CHECK_KEY(KEY_MOUSE_LEFT, key_code, is_pressed)
  CHECK_KEY(KEY_MOUSE_MIDDLE, key_code, is_pressed)
  CHECK_KEY(KEY_MOUSE_RIGHT, key_code, is_pressed)
  CHECK_KEY(KEY_MOUSE_WHEEL_FORWARD, key_code, is_pressed)
  CHECK_KEY(KEY_MOUSE_WHEEL_BACKWARD, key_code, is_pressed)
}
#endif // PLATFORM_WINDOWS
