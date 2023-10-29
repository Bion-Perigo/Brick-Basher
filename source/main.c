#include "core.h"

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
  struct transform_f transform;
  unsigned int model_id;
  unsigned int myColor_id;
  unsigned int shader_id;
  unsigned int vao_id;
  float color[4];
};

struct sprite_f init_sprite(float r, float g, float b, float a);
void draw_actor(struct sprite_f *actor);

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
  float speed = 1.f;
  struct sprite_f player = init_sprite(YELLOW);

  // Initialize Perspective
  perspective = matrix_init_perspective_f(1.f, ((float)width / height), 0.f, 2.f);

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

    if (is_key_repeat_f(KEY_LEFT)) {
      player.transform.rotation.z += speed;
    }

    if (is_key_repeat_f(KEY_RIGHT)) {
      player.transform.rotation.z += -1 * speed;
    }

    begin_drawing_p();
    clear_background_p(DARK_GRAY);

    draw_actor(&player);

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
  sprite.model_id = GL.glGetUniformLocation(sprite.shader_id, "model");
  sprite.myColor_id = GL.glGetUniformLocation(sprite.shader_id, "myColor");
  sprite.vao_id = vao;
  sprite.transform.scale.x = 1;
  sprite.transform.scale.y = 1;
  sprite.transform.scale.z = 1;
  sprite.color[0] = r;
  sprite.color[1] = g;
  sprite.color[2] = b;
  sprite.color[3] = a;

  G_LOG(LOG_INFO, "Init Sprite: Shader:%u, Proj:%u, VAO:%u", sprite.shader_id, sprite.model_id, sprite.vao_id);

  return sprite;
}

void draw_actor(struct sprite_f *sprite) {
  struct vector3_f *pos = &sprite->transform.position;
  struct vector3_f *rot = &sprite->transform.rotation;
  struct vector3_f *sca = &sprite->transform.scale;

  struct mat4_f m_translation = matrix_init_translation_f(pos->x, pos->y, pos->z);
  struct mat4_f m_rotation = matrix_init_rotation_f(rot->x, rot->y, rot->z);
  struct mat4_f m_scale = matrix_init_scale_f(sca->x, sca->y, sca->z);

  struct mat4_f m_transform = {0};
  m_transform = matrix_mult_f(m_rotation, m_scale);
  m_transform = matrix_mult_f(m_translation, m_transform);

  m_transform = matrix_mult_f(perspective, m_transform);

  GL.glBindVertexArray(sprite->vao_id);
  GL.glUniformMatrix4fv(sprite->model_id, 1, GL_FALSE, (const float *)m_transform.e);
  GL.glUniform4fv(sprite->myColor_id, 1, (const float *)sprite->color);
  GL.glUseProgram(sprite->shader_id);
  GL.glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
  GL.glBindVertexArray(0);
}
