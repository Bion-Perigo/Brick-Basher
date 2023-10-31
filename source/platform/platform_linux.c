#include "GL/glcorearb.h"

#include <stdbool.h>
#ifdef PLATFORM_LINUX
#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "GL/gl_api.h"
#include "core.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/*==================== Extern ====================*/

extern bool api_x11_init(struct window_api_p *win_api);
extern bool api_gl_init(const char *lib_name);

/*==================== Variables ====================*/

static struct window_api_p win_api;
static void *lib_egl = NULL;

static const char *egle_names[] = {
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

static struct egle_api {
  PFNEGLGETDISPLAYPROC eglGetDisplay;
  PFNEGLINITIALIZEPROC eglInitialize;
  PFNEGLBINDAPIPROC eglBindAPI;
  PFNEGLCHOOSECONFIGPROC eglChooseConfig;
  PFNEGLCREATEWINDOWSURFACEPROC eglCreateWindowSurface;
  PFNEGLCREATECONTEXTPROC eglCreateContext;
  PFNEGLMAKECURRENTPROC eglMakeCurrent;
  PFNEGLSWAPBUFFERSPROC eglSwapBuffers;
} egle_api;

static struct egl_info {
  NativeDisplayType win_display;
  NativeWindowType win_window;
  EGLDisplay *display;
  EGLConfig config;
  EGLSurface *surface;
  EGLContext *context;
} egl_info;

/*==================== Declaration ====================*/

// The window API layer will be called
void platform_linux_set_window_info(EGLNativeDisplayType dpy, EGLNativeWindowType win);

bool init_egle_api(int major, int minor, int color_bits, int depth_bits);

/*==================== Definition ====================*/

struct window_p *init_window_p(int width, int height, const char *title) {
  struct window_p *window = NULL;

  if (api_x11_init(&win_api)) {
    G_LOG(LOG_INFO, "API X11 Initialized");
    window = CALL_API(win_api.on_create_window, NULL, width, height, title);
    if (!init_egle_api(3, 3, 8, 24)) {
      G_LOG(LOG_FATAL, "EGL not Initialized");
    }
    return window;
  }

  G_LOG(LOG_FATAL, "No window API loaded");

  return NULL;
}

void close_window_p() {
  CALL_API(win_api.on_close_window, 0);
  G_LOG(LOG_INFO, "Close Window");
}

void update_window_p() {
  CALL_API(win_api.on_update_window, 0);
}

bool window_should_close_p() {
  return CALL_API(win_api.on_window_should_close, true);
}

void set_window_fullscren_p() {
  CALL_API(win_api.on_set_window_fullscreen, 0);
}

void clear_background_p(struct color_f color) {
  GL.glClearColor(color.r, color.g, color.b, color.a);
  GL.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  int width = 0, height = 0;
  get_window_size_p(&width, &height);
  GL.glViewport(0, 0, width, height);
}

void get_window_size_p(int *width, int *height) {
  CALL_API(win_api.on_get_window_size, 0, width, height);
}

int get_window_width_p() {
  int width = 0, height = 0;
  CALL_API(win_api.on_get_window_size, 0, &width, &height);
  return width;
}

int get_window_height_p() {
  int width = 0, height = 0;
  CALL_API(win_api.on_get_window_size, 0, &width, &height);
  return height;
}

void begin_drawing_p() {
  CALL_API(win_api.on_update_window, 0);
}

void end_drawing_p() {
  egle_api.eglSwapBuffers(egl_info.display, egl_info.surface);
}
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
      // G_LOG(LOG_INFO, "Get Function:%s", *func_name);
    }
    func_name++;
    index++;
  }
}

void create_log_p(enum log_level_p level, const char *context, const char *format, ...) {
  char buffer_log[BUFFER_LOG];
  const char *tag = NULL;

  switch (level) {
  case LOG_INFO: {
    tag = (const char *)"\033[0;97mLOG INFO => ";
  } break;
  case LOG_SUCCESS: {
    tag = (const char *)"\033[0;92mLOG SUCCESS => ";
  } break;
  case LOG_WARNING: {
    tag = (const char *)"\033[93mLOG WARNING => ";
  } break;
  case LOG_ERROR: {
    tag = (const char *)"\033[0;91mLOG ERROR => ";
  } break;
  case LOG_FATAL: {
    tag = (const char *)"\033[0;31mLOG FATAL => ";
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

// Isolated Functions

void platform_linux_set_window_info(EGLNativeDisplayType dpy, EGLNativeWindowType win) {
  egl_info.win_display = dpy;
  egl_info.win_window = win;
}

bool init_egle_api(int major, int minor, int color_bits, int depth_bits) {
  lib_egl = load_library_p("libEGL.so");
  if (lib_egl == NULL) {
    G_LOG(LOG_FATAL, "Lib EGL not loaded");
    return false;
  }

  get_functions_p(lib_egl, &egle_api, egle_names);

  int egl_major, egl_minor;

  egl_info.display = egle_api.eglGetDisplay(egl_info.win_display);

  if (!egle_api.eglInitialize(egl_info.display, &egl_major, &egl_minor)) {
    G_LOG(LOG_FATAL, "Init EGL: Not Initialize");
    return false;
  }

  if (!egle_api.eglBindAPI(EGL_OPENGL_API)) {
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
  if (!egle_api.eglChooseConfig(egl_info.display, attributes_list, &egl_info.config, true, &count) ||
      count != 1) {
    G_LOG(LOG_FATAL, "Init EGL: Not choose config");
    return false;
  }
  egl_info.surface =
      egle_api.eglCreateWindowSurface(egl_info.display, egl_info.config, egl_info.win_window, NULL);
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

  egl_info.context =
      egle_api.eglCreateContext(egl_info.display, egl_info.config, EGL_NO_CONTEXT, context_attribs);
  if (egl_info.context == EGL_NO_CONTEXT) {
    G_LOG(LOG_FATAL, "Init EGL: Not create context");
    return false;
  }

  if (!egle_api.eglMakeCurrent(egl_info.display, egl_info.surface, egl_info.surface, egl_info.context)) {
    G_LOG(LOG_FATAL, "Init EGL: Not make current");
    return false;
  }

  G_LOG(LOG_INFO, "EGL Initialized => EGL:%d.%d - OpenGL:%d.%d", egl_major, egl_minor, major, minor);

  api_gl_init("libGL.so");

  return true;
}
#endif // PLATFORM_LINUX
