#include "GL/glcorearb.h"
#include "core.h"

#include <GL/glcorearb.h>

int main(int argc, char *argv[]) {
  const float aspect = 1.7777;
  const int width = 1024;
  const int height = width / aspect;
  int target_fps = 60;
  bool b_show_cursor = true;

  init_window_p(width, height, "Brick-Basher");
  set_target_fps_f(target_fps);

  unsigned int VAO, VBO, IBO, TXD;
  const char *vertex = FIND_ASSET("shader/default_vertex.vert");
  const char *fragment = FIND_ASSET("shader/default_fragment.frag");
  shader_f shader = load_shader_f(vertex, fragment);

  GL.glUseProgram(shader);

  GL.glGenVertexArrays(1, &VAO);
  GL.glBindVertexArray(VAO);

  float vertices[] = {
      //[   vertices   ][      color      ]  [  Texture  ]
      0.5f,  0.5f, 0.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
      0.5f,  -0.5, 0.f, 1.f, 1.f, 1.f, 1.f, 1.f, 0.f, //
      -0.5f, -0.5, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, //
      -0.5f, 0.5f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 1.f  //
  };

  GL.glGenBuffers(1, &VBO);
  GL.glBindBuffer(GL_ARRAY_BUFFER, VBO);
  GL.glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  GL.glEnableVertexAttribArray(0);
  GL.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), NULL);
  GL.glEnableVertexAttribArray(1);
  GL.glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
  GL.glEnableVertexAttribArray(2);
  GL.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(7 * sizeof(float)));

  int index[] = {0, 1, 2, 2, 3, 0};
  GL.glGenBuffers(1, &IBO);
  GL.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
  GL.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

  struct image_f *logo = load_image_f(FIND_ASSET("texture/tests/logo.bmp"));
  GL.glGenTextures(1, &TXD);
  GL.glActiveTexture(GL_TEXTURE0);
  GL.glBindTexture(GL_TEXTURE_2D, TXD);
  GL.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, logo->width, logo->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, logo->data);
  GL.glGenerateMipmap(GL_TEXTURE_2D);
  GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

  while (!window_should_close_p()) {
    begin_frame_p();
    graphic_clear_f(DARK_GRAY);

    if (is_key_pressed_f(KEY_ENTER)) {
      b_show_cursor = !b_show_cursor;
      set_show_cursor_p(b_show_cursor);
      G_LOG(LOG_INFO, "Show Cursor:%s", (b_show_cursor) ? "True" : "false");
    }
    if (is_key_pressed_f(KEY_TAB)) {
      set_window_fullscreen_p();
    }
    if (is_key_pressed_f(KEY_ESCAPE)) {
      quit_game_p();
    }

    GL.glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    end_frame_p();
  }

  close_window_p();
}
