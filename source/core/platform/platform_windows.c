#ifdef PLATFORM_WINDOWS
#include "GL/gl_api.h"
#include "GL/glcorearb.h"
#include "GL/wglext.h"
#include "core.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <vadefs.h>
#include <winbase.h>
#include <windef.h>
#include <windows.h>

/*==================== Extern ====================*/

extern bool api_win_init(struct window_api_p *win_api);
extern void api_gl_win32_load_functions(void *opengl);
extern struct window_p *api_win32_create_dummy_window();
extern void api_win32_close_dummy_window(struct window_p *dummy_window);

/*==================== APIs ====================*/

static void *lib_gdi = NULL;
static void *lib_gl = NULL;

static const char *lib_gdi_names[] = {
    "ChoosePixelFormat", //
    "SetPixelFormat",    //
    "SwapBuffers",       //
    "\0"                 //
};

static struct api_gdi {
  int (*ChoosePixelFormat)(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd);
  BOOL (*SetPixelFormat)(HDC hdc, int format, const PIXELFORMATDESCRIPTOR *ppfd);
  BOOL (*SwapBuffers)(HDC unnamedParam1);
} api_gdi;

static const char *lib_wgl_names[] = {
    "wglCreateContext",    //
    "wglMakeCurrent",      //
    "wglGetProcAddress",   //
    "wglDeleteContext",    //
    "wglSwapLayerBuffers", //
    "\0"                   //
};

static struct api_wgl {
  HGLRC (*wglCreateContext)(HDC unnamedParam1);
  BOOL (*wglMakeCurrent)(HDC unnamedParam1, HGLRC unnamedParam2);
  PROC (*wglGetProcAddress)(LPCSTR unnamedParam1);
  BOOL (*wglDeleteContext)(HGLRC unnamedParam1);
  BOOL (*wglSwapLayerBuffers)(HDC, UINT);
} api_wgl;

/*==================== Declarations ====================*/

void win_init_console();
void win_set_console_attribute(int attribute);

/*==================== Variables ====================*/

static struct win_console {
  HANDLE h_console;
  int default_attribute;
} win_console;

static struct window_api_p win_api;
static struct window_p *main_window;
static LARGE_INTEGER time_frequency;
static LARGE_INTEGER time_begin;
static LARGE_INTEGER time_end;
static double time_target;
static double time_frametime;
static bool b_show_cursor = true;

/*==================== Definitions ====================*/

// Window =====================
struct window_p *init_window_p(int width, int height, const char *title) {
  if (main_window != NULL) {
    G_LOG(LOG_INFO, "Game window already created.");
    return main_window;
  }

  if (api_win_init(&win_api)) {
    G_LOG(LOG_INFO, "API WIN32 Initialized");
    main_window = CALL_API(win_api.on_create_window, NULL, width, height, title);
    QueryPerformanceFrequency(&time_frequency);
    if (!init_opengl_p(3, 3, 8, 24)) {
      G_LOG(LOG_FATAL, "OpenGL Not Initialized");
      return NULL;
    }
    init_graphic_g();
    return main_window;
  }
  G_LOG(LOG_FATAL, "No window API loaded");
  return NULL;
}

void close_window_p() {
  CALL_API(win_api.on_close_window, 0); // \_(´-´)_/
}

void update_window_p() {
  CALL_API(win_api.on_update_window, 0);
  update_graphic_g();
}

bool window_should_close_p() {
  return (main_window != NULL) ? main_window->should_close : true;
}

void set_window_fullscreen_p() {
  CALL_API(win_api.on_set_window_fullscreen, 0);
}

struct window_p *get_window_p() {
  return main_window;
}

void get_window_size_p(int *width, int *height) {
  *width = main_window->width;
  *height = main_window->height;
}

int get_window_width_p() {
  return main_window->width;
}

int get_window_height_p() {
  return main_window->height;
}

bool get_show_cursor_p() {
  return b_show_cursor;
}

void set_show_cursor_p(bool b_show) {
  if (b_show_cursor != b_show) {
    b_show_cursor = b_show;
    CALL_API(win_api.on_show_cursor, 0, b_show);
  }
}
void quit_game_p() {
  main_window->should_close = true;
}

// Graphic =====================
bool init_opengl_p(int major, int minor, int color_bits, int depth_bits) {
  lib_gdi = load_library_p("gdi32.dll");
  lib_gl = load_library_p("opengl32.dll");

  if (lib_gdi == NULL) {
    G_LOG(LOG_FATAL, "Lib gdi32.dll not loaded");
    return false;
  }

  if (lib_gl == NULL) {
    G_LOG(LOG_FATAL, "Lib opengl32.dll not loaded");
    return false;
  }

  get_functions_p(lib_gdi, &api_gdi, lib_gdi_names);
  get_functions_p(lib_gl, &api_wgl, lib_wgl_names);

  // Window Dummy
  struct window_p *dummy_window = api_win32_create_dummy_window();

  PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR), //  size of this pfd
      1,                             // version number
      PFD_DRAW_TO_WINDOW |           // support window
          PFD_SUPPORT_OPENGL |       // support OpenGL
          PFD_DOUBLEBUFFER,          // double buffered
      PFD_TYPE_RGBA,                 // RGBA type
      depth_bits,                    // 24-bit color depth
      0,
      0,
      0,
      0,
      0,
      0, // color bits ignored
      0, // no alpha buffer
      0, // shift bit ignored
      0, // no accumulation buffer
      0,
      0,
      0,
      0,              // accum bits ignored
      color_bits,     // 32-bit z-buffer
      0,              // no stencil buffer
      0,              // no auxiliary buffer
      PFD_MAIN_PLANE, // main layer
      0,              // reserved
      0,
      0,
      0 // layer masks ignored
  };

  int pixel_format = api_gdi.ChoosePixelFormat(dummy_window->display, &pfd);

  if (!pixel_format) {
    G_LOG(LOG_FATAL, "Not Choose Pixel Format");
    return FALSE;
  }
  if (!api_gdi.SetPixelFormat(dummy_window->display, pixel_format, &pfd)) {
    G_LOG(LOG_FATAL, "Not Set Pixel Format");
  }

  HGLRC rc;
  rc = api_wgl.wglCreateContext(dummy_window->display);
  if (!rc) {
    G_LOG(LOG_FATAL, "WGL: HGLRC Invalid");
    return FALSE;
  }

  if (!api_wgl.wglMakeCurrent(dummy_window->display, rc)) {
    G_LOG(LOG_FATAL, "WGL: Not Make Current");
    return FALSE;
  }

  PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB =
      (PFNWGLCHOOSEPIXELFORMATARBPROC)api_wgl.wglGetProcAddress("wglChoosePixelFormatARB");
  PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
      (PFNWGLCREATECONTEXTATTRIBSARBPROC)api_wgl.wglGetProcAddress("wglCreateContextAttribsARB");

  api_wgl.wglMakeCurrent(0, 0);
  api_wgl.wglDeleteContext(rc);
  api_win32_close_dummy_window(dummy_window);

  int pixel_format_count;
  int pixel_format_attrib_list[] = {WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,   WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                                    WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,   WGL_COLOR_BITS_ARB,     color_bits,
                                    WGL_DEPTH_BITS_ARB,     depth_bits};

  wglChoosePixelFormatARB(
      main_window->display, pixel_format_attrib_list, NULL, 1, &pixel_format, (UINT *)&pixel_format_count);
  if (pixel_format_count < 0) {
    G_LOG(LOG_FATAL, "WGL: Not Create Modern Context")
    return FALSE;
  }

  if (!api_gdi.SetPixelFormat(main_window->display, pixel_format, &pfd)) {
    G_LOG(LOG_FATAL, "WGL: Not Set Pixel Format in Game Window")
    return FALSE;
  }

  const int context_attribs[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB,
      major,
      WGL_CONTEXT_MINOR_VERSION_ARB,
      minor,
      WGL_CONTEXT_FLAGS_ARB,
#ifdef DEBUG_MODE
      WGL_CONTEXT_DEBUG_BIT_ARB |
#endif // DEBUG_MODE
          WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0};

  rc = wglCreateContextAttribsARB(main_window->display, 0, context_attribs);
  if (!rc) {
    G_LOG(LOG_FATAL, "WGL: Not Create Context ARB");
    return FALSE;
  }

  if (!api_wgl.wglMakeCurrent(main_window->display, rc)) {
    G_LOG(LOG_FATAL, "WGL: Not Make Current ARB");
    return FALSE;
  }

  G_LOG(LOG_INFO, "WGL: Create Context => OpenGL:%d.%d", major, minor);

  api_gl_win32_load_functions(lib_gl);

  return true;
}

void begin_frame_p() {
  QueryPerformanceCounter(&time_begin);
  update_window_p();
}

void end_frame_p() {
  api_gdi.SwapBuffers(main_window->display);
  QueryPerformanceCounter(&time_end);
  long long elapsed = time_end.QuadPart - time_begin.QuadPart;
  time_frametime = (double)elapsed / (double)time_frequency.QuadPart;

  if (time_target > 0.f) {
    while (time_frametime < time_target) {
      QueryPerformanceCounter(&time_end);
      elapsed = time_end.QuadPart - time_begin.QuadPart;
      time_frametime = (float)elapsed / (double)time_frequency.QuadPart;
    }
  }
}

double get_time_p() {
  LARGE_INTEGER counter;
  double time = 0.f;
  QueryPerformanceCounter(&counter);
  time = counter.QuadPart;
  time = (float)(time / (double)(time_frequency.QuadPart));
  return time;
}

void set_target_fps_p(int max_fps) {
  time_target = (max_fps > 0) ? 1.f / max_fps : 0;
}

double get_frametime_p() {
  return time_frametime;
}

int get_framerate_p() {
  if (1.f / time_frametime < 40) {
    G_LOG(LOG_INFO, "FPS:%d, %f", 1.f / time_frametime, time_frametime);
  }
  return 1.f / time_frametime;
}

// Library =====================
void *load_library_p(const char *library_name) {
  if (library_name == NULL) {
    G_LOG(LOG_INFO, "Load Library: Received NULL");
    return NULL;
  }
  void *lib = LoadLibrary(library_name);

  if (lib == NULL) {
    G_LOG(LOG_INFO, "Load Library: Not Loaded:%s", library_name);
    return NULL;
  }

  G_LOG(LOG_INFO, "Load Library:%s", library_name);

  return lib;
}

bool free_library_p(void *library) {
  if (library == NULL) {
    G_LOG(LOG_INFO, "Free Library: Received NULL");
    return false;
  }
  FreeLibrary(library);
  G_LOG(LOG_INFO, "Free Library: Unloaded");

  return true;
}

void *get_function_p(void *library, const char *name) {
  if (library == NULL || name == NULL) {
    G_LOG(LOG_INFO, "Get Function: Received Null parameters");
    return NULL;
  }

  return GetProcAddress(library, name);
}

void get_functions_p(void *library, void *api, const char **names) {
  const char **func_name = names;
  void *object = api;
  int index = 0;
  void *function = NULL;

  while (**func_name) {
    function = get_function_p(library, *func_name);
    if (function != NULL) {
      void *address = (void *)object + (index * sizeof(char *));
      copy_memory_f((void *)address, (char *)&function, sizeof(char *));
    } else {
      G_LOG(LOG_ERROR, "Not Loaded:%s", *func_name);
    }
    func_name++;
    index++;
  }
}

void create_log_p(enum log_level_p level, const char *context, const char *format, ...) {
  if (win_console.h_console == NULL) {
    win_init_console();
  }
  char buffer_log[BUFFER_LOG] = {0};
  const char *tag = NULL;

  enum _color {
    WIN_COLOR_WHITE = 15,
    WIN_COLOR_GREEN = 10,
    WIN_COLOR_YELLOW = 14,
    WIN_COLOR_RED = 12,
    WIN_COLOR_DARK_RED = 4,
  } win_color = WIN_COLOR_WHITE;

  switch (level) {
  case LOG_INFO:
    tag = (const char *)"INFO => ";
    win_color = WIN_COLOR_WHITE;
    break;
  case LOG_SUCCESS:
    tag = (const char *)"SUCCESS => ";
    win_color = WIN_COLOR_GREEN;
    break;
  case LOG_WARNING:
    tag = (const char *)"WARNING => ";
    win_color = WIN_COLOR_YELLOW;
    break;
  case LOG_ERROR:
    tag = (const char *)"ERROR => ";
    win_color = WIN_COLOR_RED;
    break;
  case LOG_FATAL:
    tag = (const char *)"FATAL => ";
    win_color = WIN_COLOR_DARK_RED;
    break;
  }

  strcpy(buffer_log, tag);
  strcat(buffer_log, format);
  if (level != LOG_INFO) {
    strcat(buffer_log, context);
  }
  strcat(buffer_log, "\n");

  win_set_console_attribute(win_color);
  va_list args;
  va_start(args, format);
  vprintf(buffer_log, args);
  va_end(args);
  win_set_console_attribute(win_console.default_attribute);
}

void win_init_console() {
  win_console.h_console = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO console_info;
  GetConsoleScreenBufferInfo(win_console.h_console, &console_info);
  win_console.default_attribute = console_info.wAttributes;
}

void win_set_console_attribute(int attribute) {
  SetConsoleTextAttribute(win_console.h_console, attribute);
}

#endif // PLATFORM_WINDOWS
