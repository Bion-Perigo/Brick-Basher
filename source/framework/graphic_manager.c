#include "GL/gl_api.h"
#include "GL/glcorearb.h"
#include "core.h"

void graphic_init_f() {
  GL.glEnable(GL_DEPTH_TEST);
  GL.glEnable(GL_BLEND);
  GL.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void graphic_update_f() {
}

void graphic_clear_f(struct color_f color) {
  GL.glClearColor(color.r, color.g, color.b, color.a);
  GL.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  GL.glViewport(0, 0, get_window_width_p(), get_window_height_p());
}
