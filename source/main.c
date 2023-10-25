#include "GL/gl_api.h"
#include "GL/glcorearb.h"
#include "core.h"

#include <GL/glcorearb.h>
#include <X11/X.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

// Test
struct vector3_f {
  float x;
  float y;
  float z;
};

struct transform_f {
  struct vector3_f position;
  struct vector3_f rotation;
  struct vector3_f scale;
};

struct sprite_f {
  unsigned int projection;
  unsigned int myColor_id;
  unsigned int shader_id;
  unsigned int vao_id;
  float color[4];
};

struct actor {
  float speed;
  float angle;
  struct transform_f transform;
  struct sprite_f sprite;
  struct mat4_f model;
};

struct sprite_f init_sprite(float r, float g, float b, float a);
void update_player_controller(struct actor *actor);
void set_actor_transform(struct actor *actor);
void update_actor(struct actor *actor);
void render_sprite(struct actor *actor);

struct mat4_f perspective = {0};

// Test

int main(int argc, char *argv[]) {
  G_LOG(LOG_INFO, "Hello World");
  G_LOG(LOG_SUCCESS, "Hello World");
  G_LOG(LOG_WARNING, "Hello World");
  G_LOG(LOG_ERROR, "Hello World");
  G_LOG(LOG_FATAL, "Hello World");

  int width = 1024;
  int height = 576;

  init_window_p(width, height, "Brick-Basher");

  // Initialize Player
  struct actor player = {0};
  player.sprite = init_sprite(YELLOW);
  player.transform.scale.x = 1;
  player.transform.scale.y = 1;
  player.transform.scale.z = 1;
  player.speed = 0.01f;
  player.angle = 1.f;

  // Initialize Enemy
  struct actor enemy = {0};
  enemy.sprite = init_sprite(RED);
  enemy.transform.scale.x = 1;
  enemy.transform.scale.y = 1;
  enemy.transform.scale.z = 1;
  enemy.speed = 0.01f;
  enemy.angle = 1.f;

  enemy.transform.position.x = 2.f;
  enemy.transform.position.z = -2.f;

  // Initialize Perspective
  perspective = matrix_init_perspective_f(1.f, ((float)width / height), 0.f, 2.f);

  struct actor *actors[2] = {&player, &enemy};

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
    update_player_controller(&player);

    // Draw
    begin_drawing_p();
    clear_background_p(DARK_GRAY);

    for (int i = 0; i < 2; i++) { // Update all actors
      update_actor(actors[i]);
    }

    end_drawing_p();
  }

  close_window_p();

  return 0;
}

struct sprite_f init_sprite(float r, float g, float b, float a) {
  struct sprite_f sprite = {0};
  unsigned int vao, vbo, ibo;

  GL.glGenVertexArrays(1, &vao);
  GL.glBindVertexArray(vao);

  float vertices[] = {
      -0.5f, -0.5f, 0.f, WHITE, // Botton Left
      0.5f,  -0.5f, 0.f, WHITE, // Botton Right
      0.5f,  0.5f,  0.f, WHITE, // Top Right
      -0.5f, 0.5f,  0.f, WHITE  // Top Left
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

  const char *vertex = FIND_ASSET("shader/default_vertex.vert");
  const char *fragment = FIND_ASSET("shader/default_fragment.frag");

  sprite.shader_id = load_shader_f(vertex, fragment);
  sprite.projection = GL.glGetUniformLocation(sprite.shader_id, "proj");
  sprite.myColor_id = GL.glGetUniformLocation(sprite.shader_id, "myColor");
  sprite.vao_id = vao;
  sprite.color[0] = r;
  sprite.color[1] = g;
  sprite.color[2] = b;
  sprite.color[3] = a;

  G_LOG(LOG_INFO, "Init Sprite: Shader:%u, Proj:%u, VAO:%u", sprite.shader_id, sprite.projection, sprite.vao_id);

  return sprite;
}

void update_player_controller(struct actor *actor) {
  if (is_key_repeat_f(KEY_W)) {
    actor->transform.position.y += actor->speed;
  }

  if (is_key_repeat_f(KEY_S)) {
    actor->transform.position.y -= actor->speed;
  }

  if (is_key_repeat_f(KEY_A)) {
    actor->transform.position.x -= actor->speed;
  }
  if (is_key_repeat_f(KEY_D)) {
    actor->transform.position.x += actor->speed;
  }
  if (is_key_repeat_f(KEY_R)) {
    actor->transform.position.z -= actor->speed;
  }
  if (is_key_repeat_f(KEY_F)) {
    actor->transform.position.z += actor->speed;
  }

  if (is_key_repeat_f(KEY_LEFT)) {
    actor->transform.rotation.y += -1 * actor->angle;
  }
  if (is_key_repeat_f(KEY_RIGHT)) {
    actor->transform.rotation.y += actor->angle;
  }

  if (is_key_repeat_f(KEY_UP)) {
    actor->transform.rotation.x += -1 * actor->angle;
  }
  if (is_key_repeat_f(KEY_DOWN)) {
    actor->transform.rotation.x += actor->angle;
  }

  if (is_key_repeat_f(KEY_E)) {
    actor->transform.rotation.z += -1 * actor->angle;
  }
  if (is_key_repeat_f(KEY_Q)) {
    actor->transform.rotation.z += actor->angle;
  }
}

void set_actor_transform(struct actor *actor) {
  struct vector3_f *pos = &actor->transform.position;
  struct vector3_f *rot = &actor->transform.rotation;
  struct vector3_f *sca = &actor->transform.scale;

  struct mat4_f m_translation = matrix_init_translation_f(pos->x, pos->y, pos->z);
  struct mat4_f m_rotation = matrix_init_rotation_f(rot->x, rot->y, rot->z);
  struct mat4_f m_scale = matrix_init_scale_f(sca->x, sca->y, sca->z);

  struct mat4_f m_transform = {0};
  m_transform = matrix_mult_f(m_rotation, m_scale);
  m_transform = matrix_mult_f(m_translation, m_transform);

  actor->model = matrix_mult_f(perspective, m_transform);

  GL.glUniformMatrix4fv(actor->sprite.projection, 1, 0, (const float *)actor->model.e);
  GL.glUniform4fv(actor->sprite.myColor_id, 1, (const float *)actor->sprite.color);
}

void update_actor(struct actor *actor) {
  set_actor_transform(actor);
  render_sprite(actor);
}

void render_sprite(struct actor *actor) {
  GL.glUseProgram(actor->sprite.shader_id);
  GL.glBindVertexArray(actor->sprite.vao_id);
  GL.glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
  GL.glBindVertexArray(0);
}
