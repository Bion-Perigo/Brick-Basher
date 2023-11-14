#ifdef PLATFORM_LINUX
#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "GL/gl_api.h"
#include "GL/glcorearb.h"
#include "core.h"

#include <bits/time.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/*==================== Extern ====================*/

extern bool api_x11_init(struct window_api_p *win_api);
extern bool api_gl_init(const char *lib_name);

/*==================== Variables ====================*/

static struct window_api_p win_api;
static void *lib_egl = NULL;

static const char *egl_names[] = {
    "eglGetDisplay",          //
    "eglInitialize",          //
    "eglBindAPI",             //
    "eglChooseConfig",        //
    "eglCreateWindowSurface", //
    "eglCreateContext",       //
    "eglMakeCurrent",         //
    "eglSwapBuffers",         //
    "\0"                      //
};

static struct egl_api {
  PFNEGLGETDISPLAYPROC eglGetDisplay;
  PFNEGLINITIALIZEPROC eglInitialize;
  PFNEGLBINDAPIPROC eglBindAPI;
  PFNEGLCHOOSECONFIGPROC eglChooseConfig;
  PFNEGLCREATEWINDOWSURFACEPROC eglCreateWindowSurface;
  PFNEGLCREATECONTEXTPROC eglCreateContext;
  PFNEGLMAKECURRENTPROC eglMakeCurrent;
  PFNEGLSWAPBUFFERSPROC eglSwapBuffers;
} egl_api;

static struct egl_info {
  EGLDisplay *display;
  EGLConfig config;
  EGLSurface *surface;
  EGLContext *context;
} egl_info;

static struct window_p *main_window;

/*==================== Declaration ====================*/

/*==================== Definition ====================*/

// Window =========================================

struct window_p *init_window_p(int width, int height, const char *title) {
  if (main_window != NULL) {
    G_LOG(LOG_INFO, "Game window already created.");
    return main_window;
  }

  if (api_x11_init(&win_api)) {
    G_LOG(LOG_INFO, "API X11 Initialized");
    main_window = CALL_API(win_api.on_create_window, NULL, width, height, title);
    if (!init_opengl_p(3, 3, 8, 8)) {
      G_LOG(LOG_FATAL, "EGL not Initialized");
    }
    return main_window;
  }

  G_LOG(LOG_FATAL, "No window API loaded");

  return NULL;
}

void close_window_p() {
  CALL_API(win_api.on_close_window, 0);
}

void update_window_p() {
  CALL_API(win_api.on_update_window, 0);
}

bool window_should_close_p() {
  return main_window->should_close;
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

void set_show_cursor_p(bool b_show) {
  CALL_API(win_api.on_show_cursor, 0, b_show);
}

void quit_game_p() {
  main_window->should_close = true;
}
float get_time_p() {
  struct timespec spec = {0};
  clock_gettime(CLOCK_MONOTONIC, &spec);
  float time = spec.tv_sec + spec.tv_nsec / 1000000000.0;
  time /= 1000;
  return time;
}

void wait_time_p(double time) {
  if (time <= 0) {
    return;
  }
  struct timespec req = {0};
  time_t sec = time;
  long n_sec = (time - sec) * 1000000000L;
  req.tv_sec = sec;
  req.tv_nsec = n_sec;

  while (nanosleep(&req, &req) == 1) {
    continue;
  }
}

// Graphic =========================================

bool init_opengl_p(int major, int minor, int color_bits, int depth_bits) {
  lib_egl = load_library_p("libEGL.so");
  if (lib_egl == NULL) {
    G_LOG(LOG_FATAL, "Lib EGL not loaded");
    return false;
  }

  get_functions_p(lib_egl, &egl_api, egl_names);

  int egl_major, egl_minor;

  egl_info.display = egl_api.eglGetDisplay(main_window->display);

  if (!egl_api.eglInitialize(egl_info.display, &egl_major, &egl_minor)) {
    G_LOG(LOG_FATAL, "Init EGL: Not Initialize");
    return false;
  }

  if (!egl_api.eglBindAPI(EGL_OPENGL_API)) {
    G_LOG(LOG_FATAL, "Int EGL: Not bind to OpenGL");
    return false;
  }

  EGLint attributes_list[] = {
      EGL_RENDERABLE_TYPE, //
      EGL_OPENGL_BIT,      //
      EGL_RED_SIZE,        //
      color_bits,          //
      EGL_GREEN_SIZE,      //
      color_bits,          //
      EGL_BLUE_SIZE,       //
      color_bits,          //
      EGL_DEPTH_SIZE,      //
      depth_bits,          //
      EGL_STENCIL_SIZE,    //
      color_bits,          //
      EGL_NONE             //
  };

  EGLint count;
  if (!egl_api.eglChooseConfig(egl_info.display, attributes_list, &egl_info.config, true, &count) || count != 1) {
    G_LOG(LOG_FATAL, "Init EGL: Not choose config");
    return false;
  }
  egl_info.surface = egl_api.eglCreateWindowSurface(egl_info.display, egl_info.config, main_window->window, NULL);
  if (egl_info.surface == EGL_NO_SURFACE) {
    G_LOG(LOG_FATAL, "Init EGL: Not create window surface");
    return false;
  }

  EGLint context_attribs[] = {
      EGL_CONTEXT_MAJOR_VERSION,           //
      major,                               //
      EGL_CONTEXT_MINOR_VERSION,           //
      minor,                               //
      EGL_CONTEXT_OPENGL_PROFILE_MASK,     //
      EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT, //
#ifdef DEBUG_MODE
      EGL_CONTEXT_OPENGL_DEBUG, //
      EGL_TRUE,
#endif
      EGL_NONE //
  };

  egl_info.context = egl_api.eglCreateContext(egl_info.display, egl_info.config, EGL_NO_CONTEXT, context_attribs);
  if (egl_info.context == EGL_NO_CONTEXT) {
    G_LOG(LOG_FATAL, "Init EGL: Not create context");
    return false;
  }

  if (!egl_api.eglMakeCurrent(egl_info.display, egl_info.surface, egl_info.surface, egl_info.context)) {
    G_LOG(LOG_FATAL, "Init EGL: Not make current");
    return false;
  }

  G_LOG(LOG_INFO, "API EGL Initialized => EGL:%d.%d - OpenGL:%d.%d", egl_major, egl_minor, major, minor);

  api_gl_init("libGL.so");

  return true;
}

void begin_frame_p() {
  begin_time_f();
  CALL_API(win_api.on_update_window, 0);
}

void end_frame_p() {
  egl_api.eglSwapBuffers(egl_info.display, egl_info.surface);
  end_time_f();
}

// Library =========================================

void *load_library_p(const char *library_name) {
  if (library_name == NULL) {
    G_LOG(LOG_INFO, "Load Library: Received NULL");
    return NULL;
  }
  void *lib = dlopen(library_name, RTLD_LAZY);

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
  dlclose(library);
  G_LOG(LOG_INFO, "Free Library: Unloaded");

  return true;
}

void *get_function_p(void *library, const char *name) {
  if (library == NULL || name == NULL) {
    G_LOG(LOG_INFO, "Get Function: Received Null parameters");
    return NULL;
  }

  return dlsym(library, name);
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
    }
    func_name++;
    index++;
  }
}

// Log =========================================

void create_log_p(enum log_level_p level, const char *context, const char *format, ...) {
  char buffer_log[BUFFER_LOG];
  const char *tag = NULL;

  switch (level) {
  case LOG_INFO: {
    tag = (const char *)"\033[0;97mINFO => ";
  } break;
  case LOG_SUCCESS: {
    tag = (const char *)"\033[0;92mSUCCESS => ";
  } break;
  case LOG_WARNING: {
    tag = (const char *)"\033[93mWARNING => ";
  } break;
  case LOG_ERROR: {
    tag = (const char *)"\033[0;91mERROR => ";
  } break;
  case LOG_FATAL: {
    tag = (const char *)"\033[0;31mFATAL => ";
  } break;
  }

  strcpy(buffer_log, tag);
  strcat(buffer_log, format);
  if (level != LOG_INFO) {
    strcat(buffer_log, context);
  }
  strcat(buffer_log, "\033[0m\n");

  va_list args;
  va_start(args, format);
  vprintf(buffer_log, args);
  va_end(args);
}

#endif // PLATFORM_LINUX
