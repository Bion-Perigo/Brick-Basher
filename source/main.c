#include "GL/glcorearb.h"
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
  struct mat4_f model;
};

struct sprite_f {
  struct transform_f transform;
  unsigned int model_id;
  unsigned int myColor_id;
  unsigned int shader_id;
  unsigned int vao_id;
  struct color_f color;
};

struct camera_f {
  struct transform_f transform;
  unsigned int proj_id;
  unsigned int view_id;
  struct mat4_f proj;
  struct mat4_f view;
  float fov;
  float aspect;
  float z_near;
  float z_far;
};

struct camera_f init_camera(float fov, float aspect, float z_near, float z_far);
void update_camera(struct camera_f *camera);
struct sprite_f init_sprite(struct color_f color);
void draw_actor(struct sprite_f *actor);

struct camera_f main_camera = {0};
shader_f default_shader = 0;

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

  // Load Defult Shader
  const char *vertex = FIND_ASSET("shader/default_vertex.vert");
  const char *fragment = FIND_ASSET("shader/default_fragment.frag");
  default_shader = load_shader_f(vertex, fragment);

  // Initialize Perspective
  main_camera = init_camera(1.f, ((float)width / height), 0.f, 2.f);

  // Initialize Player
  float speed = 0.05f;
  struct sprite_f player = init_sprite(YELLOW);

  while (!window_should_close_p()) {
    // Update
    if (is_key_pressed_f(KEY_TAB)) {
      G_LOG(LOG_INFO, "Toggle Fullscreen");
      set_window_fullscreen_p();
    }

    if (is_key_pressed_f(KEY_ESCAPE)) {
      G_LOG(LOG_WARNING, "Quit Game");
      break;
    }

    if (is_key_repeat_f(KEY_S)) {
      main_camera.transform.position.z += speed;
    }

    if (is_key_repeat_f(KEY_W)) {
      main_camera.transform.position.z += -1 * speed;
    }

    if (is_key_repeat_f(KEY_D)) {
      main_camera.transform.position.x += speed;
    }

    if (is_key_repeat_f(KEY_A)) {
      main_camera.transform.position.x += -1 * speed;
    }

    begin_drawing_p();
    clear_background_p(DARK_GRAY);
    update_camera(&main_camera);

    draw_actor(&player);

    end_drawing_p();
  }

  close_window_p();

  return 0;
}

struct camera_f init_camera(float fov, float aspect, float z_near, float z_far) {
  struct camera_f camera = {0};
  camera.fov = fov;
  camera.aspect = aspect;
  camera.z_near = z_near;
  camera.z_far = z_far;
  camera.proj = matrix_init_perspective_f(camera.fov, camera.aspect, camera.z_near, camera.z_far);
  camera.proj_id = GL.glGetUniformLocation(default_shader, "proj");
  camera.view_id = GL.glGetUniformLocation(default_shader, "view");

  camera.transform.scale.x = 1.f;
  camera.transform.scale.y = 1.f;
  camera.transform.scale.z = 1.f;

  return camera;
}

void update_camera(struct camera_f *camera) {
  int viewport[4] = {0};
  GL.glGetIntegerv(GL_VIEWPORT, viewport);
  camera->aspect = (float)viewport[2] / viewport[3];
  camera->proj = matrix_init_perspective_f(camera->fov, camera->aspect, camera->z_near, camera->z_far);

  struct vector3_f *cam_pos = &camera->transform.position;

  struct mat4_f m_transform = matrix_init_translation_f(cam_pos->x, cam_pos->y, cam_pos->z);
  struct mat4_f m_view = matrix_inverse_f(m_transform);

  camera->transform.model = m_transform;
  camera->view = m_view;

  GL.glUniformMatrix4fv(camera->proj_id, 1, GL_FALSE, (const float *)camera->proj.e);
  GL.glUniformMatrix4fv(camera->view_id, 1, GL_FALSE, (const float *)camera->view.e);
}

struct sprite_f init_sprite(struct color_f color) {
  struct sprite_f sprite = {0};
  unsigned int vao, vbo, ibo;

  GL.glGenVertexArrays(1, &vao);
  GL.glBindVertexArray(vao);

  float vertices[] = {
      -0.5f, -0.5f, 0.f, 1.f, 1.f, 1.f, 1.f, // Botton Left
      0.5f,  -0.5f, 0.f, 1.f, 1.f, 1.f, 1.f, // Botton Right
      0.5f,  0.5f,  0.f, 1.f, 1.f, 1.f, 1.f, // Top Right
      -0.5f, 0.5f,  0.f, 1.f, 1.f, 1.f, 1.f  // Top Left
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

  sprite.shader_id = default_shader;
  sprite.model_id = GL.glGetUniformLocation(sprite.shader_id, "model");
  sprite.myColor_id = GL.glGetUniformLocation(sprite.shader_id, "myColor");
  sprite.vao_id = vao;
  sprite.transform.scale.x = 1;
  sprite.transform.scale.y = 1;
  sprite.transform.scale.z = 1;
  sprite.color.r = color.r;
  sprite.color.g = color.g;
  sprite.color.b = color.b;
  sprite.color.a = color.a;

  G_LOG(LOG_INFO, "Init Sprite: Shader:%u, Model:%u, VAO:%u", sprite.shader_id, sprite.model_id, sprite.vao_id);

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

  m_transform = matrix_mult_f(main_camera.view, m_transform);

  GL.glBindVertexArray(sprite->vao_id);

  GL.glUniformMatrix4fv(sprite->model_id, 1, GL_FALSE, (const float *)m_transform.e);
  GL.glUniform4fv(sprite->myColor_id, 1, (const float *)&sprite->color);

  GL.glUseProgram(sprite->shader_id);
  GL.glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
  GL.glBindVertexArray(0);
}
