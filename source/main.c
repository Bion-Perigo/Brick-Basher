#include "GL/gl_api.h"
#include "GL/glcorearb.h"
#include "core.h"

#include <GL/glcorearb.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

struct vector3 {
  float x;
  float y;
  float z;
};

struct actor {
  bool is_dirty;
  float speed;
  float angle;
  struct vector3 position;
  struct vector3 rotation;
  struct vector3 scale;
  struct mat4_f model;
};

void set_actor_transform(struct actor *actor);
void update_control(struct actor *actor);
void init_rect();

struct mat4_f perspective = {0};
GLuint uniform_proj = 0;

int main(int argc, char *argv[]) {
  G_LOG(LOG_INFO, "Hello World");
  G_LOG(LOG_SUCCESS, "Hello World");
  G_LOG(LOG_WARNING, "Hello World");
  G_LOG(LOG_ERROR, "Hello World");
  G_LOG(LOG_FATAL, "Hello World");

  int width = 1024;
  int height = 576;

  init_window_p(width, height, "Brick-Basher");

  init_rect();

  const char *vertex = FIND_ASSET("shader/default_vertex.vert");
  const char *fragment = FIND_ASSET("shader/default_fragment.frag");

  shader_f shader = load_shader_f(vertex, fragment);
  GL.glUseProgram(shader);

  perspective = matrix_init_perspective_f(1.0f, ((float)width / (float)height), 0.0f, 2.0f);
  uniform_proj = GL.glGetUniformLocation(shader, "proj");

  struct actor quad = {0};
  quad.scale.x = 1;
  quad.scale.y = 1;
  quad.scale.z = 1;

  quad.is_dirty = false;
  quad.speed = 0.01f;
  quad.angle = 1.f;

  while (!window_should_close_p()) {
    // Update
    if (is_key_pressed_f(KEY_TAB)) {
      G_LOG(LOG_INFO, "Toggle Fullscreen");
      set_window_fullscren_p();
    }

    if (is_key_pressed_f(KEY_ESCAPE)) {
      G_LOG(LOG_WARNING, "Quit Game");
      break;
    }

    // Inputs
    update_control(&quad);

    // Draw
    begin_drawing_p();
    clear_background_p(DARK_GRAY);

    set_actor_transform(&quad);

    GL.glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    end_drawing_p();
  }

  close_window_p();

  return 0;
}

void set_actor_transform(struct actor *actor) {
  struct mat4_f m_translation =
      matrix_init_translation_f(actor->position.x, actor->position.y, actor->position.z);
  struct mat4_f m_rotation = matrix_init_rotation_f(actor->rotation.x, actor->rotation.y, actor->rotation.z);
  struct mat4_f m_scale = matrix_init_scale_f(actor->scale.x, actor->scale.y, actor->scale.z);

  struct mat4_f m_transform = matrix_mult_f(m_rotation, m_scale);
  m_transform = matrix_mult_f(m_translation, m_transform);
  actor->model = matrix_mult_f(perspective, m_transform);

  GL.glUniformMatrix4fv(uniform_proj, 1, 0, (const float *)actor->model.e);
}

void update_control(struct actor *actor) {
  if (is_key_repeat_f(KEY_W)) {
    actor->position.y += actor->speed;
    actor->is_dirty = true;
    G_LOG(LOG_INFO, "KEY_W Pressed:%f", actor->position.y);
  }

  if (is_key_repeat_f(KEY_S)) {
    actor->position.y -= actor->speed;
    G_LOG(LOG_INFO, "KEY_S Pressed:%f", actor->position.y);
  }

  if (is_key_repeat_f(KEY_A)) {
    G_LOG(LOG_INFO, "KEY_A Pressed");
    actor->position.x -= actor->speed;
  }
  if (is_key_repeat_f(KEY_D)) {
    G_LOG(LOG_INFO, "KEY_S Pressed");
    actor->position.x += actor->speed;
  }
  if (is_key_repeat_f(KEY_R)) {
    G_LOG(LOG_INFO, "KEY_R Pressed");
    actor->position.z -= actor->speed;
  }
  if (is_key_repeat_f(KEY_F)) {
    G_LOG(LOG_INFO, "KEY_F Pressed");
    actor->position.z += actor->speed;
  }

  if (is_key_repeat_f(KEY_LEFT)) {
    actor->rotation.y += -1 * actor->angle;
    G_LOG(LOG_INFO, "KEY_LEFT Repeat:Y%f", actor->rotation.y);
  }
  if (is_key_repeat_f(KEY_RIGHT)) {
    actor->rotation.y += actor->angle;
    G_LOG(LOG_INFO, "KEY_RIGHT Repeat:Y%f", actor->rotation.y);
  }

  if (is_key_repeat_f(KEY_UP)) {
    actor->rotation.x += -1 * actor->angle;
    G_LOG(LOG_INFO, "KEY_UP Repeat:X%f", actor->rotation.x);
  }
  if (is_key_repeat_f(KEY_DOWN)) {
    actor->rotation.x += actor->angle;
    G_LOG(LOG_INFO, "KEY_DOWN Repeat:X%f", actor->rotation.x);
  }

  if (is_key_repeat_f(KEY_E)) {
    actor->rotation.z += -1 * actor->angle;
    G_LOG(LOG_INFO, "KEY_UP Repeat:E%f", actor->rotation.z);
  }
  if (is_key_repeat_f(KEY_Q)) {
    actor->rotation.z += actor->angle;
    G_LOG(LOG_INFO, "KEY_DOWN Repeat:Q%f", actor->rotation.z);
  }
}

void init_rect() {

  unsigned int vao, vbo, ibo;

  GL.glGenVertexArrays(1, &vao);
  GL.glBindVertexArray(vao);

  float vertices[] = {
      -0.5f, -0.5f, 0.f, RED,   // Botton Left
      0.5f,  -0.5f, 0.f, GREEN, // Botton Right
      0.5f,  0.5f,  0.f, BLUE,  // Top Right
      -0.5f, 0.5f,  0.f, YELLOW // Top Left
  };

  GL.glGenBuffers(1, &vbo);
  GL.glBindBuffer(GL_ARRAY_BUFFER, vbo);

  GL.glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  GL.glEnableVertexAttribArray(0);
  GL.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), NULL);
  GL.glEnableVertexAttribArray(1);
  GL.glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *)(3 * sizeof(float)));

  int index[] = {0, 1, 2, 2, 3, 0};

  GL.glGenBuffers(1, &ibo);
  GL.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  GL.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);
}
