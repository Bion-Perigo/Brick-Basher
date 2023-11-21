#include "GL/gl_api.h"
#include "GL/glcorearb.h"
#include "core.h"

#define Z_NEAR 0.f
#define Z_FAR  10.f

static enum camera_mode camera_mode = CAMERA_ORTHOGRAPHIC;
static unsigned int default_shader = 0;

void init_graphic_g() {
  const char *source_vertex = FIND_ASSET("shader/default_vertex.vert");
  const char *source_fragment = FIND_ASSET("shader/default_fragment.frag");
  default_shader = load_shader_g(source_vertex, source_fragment);
  if (default_shader != 0) {
    G_LOG(LOG_INFO, "GRAPHIC: Loaded Default Shader");
  } else {
    G_LOG(LOG_INFO, "GRAPHIC: Not Loaded Default Shader");
  }

  GL.glEnable(GL_DEPTH_TEST);
  GL.glEnable(GL_BLEND);
  GL.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void update_graphic_g() {
}

void clear_background_g(struct color_f color) {
  GL.glClearColor(color.r, color.g, color.b, color.a);
  GL.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  GL.glViewport(0, 0, get_window_width_p(), get_window_height_p());
}

int get_camera_mode_g() {
  return camera_mode;
}
void set_camera_mode_g(enum camera_mode mode) {
  camera_mode = mode;
}

struct mat4_f get_projection_matrix_g() {
  struct mat4_f cam_proj = matrix_identity_f();
  if (camera_mode == CAMERA_ORTHOGRAPHIC) {
    cam_proj = matrix_init_ortho_f(0, 100, 100, 0, Z_NEAR, Z_FAR);
  }

  return cam_proj;
}

struct texture_f create_texture_g(struct image_f *image) {
  struct texture_f texture = {0};
  struct image_f *img = {0};
  unsigned int txd = 0;
  GLenum color_format = 0;
  GLenum color_type = 0;

  img = image;

  if (image == NULL) {
    G_LOG(LOG_INFO, "GRAPHIC: Image is Null");
    img = load_image_default_f();
  }

  switch (img->format) {
  case 32: {
    color_format = GL_RGBA;
    color_type = GL_UNSIGNED_BYTE;
  } break;
  case 24: {
    color_format = GL_RGB;
    color_type = GL_UNSIGNED_BYTE;
  } break;
  case 16: {
    color_format = GL_RGB;
    color_type = GL_UNSIGNED_SHORT_5_6_5;
  } break;
  }

  GL.glGenTextures(1, &txd);
  GL.glActiveTexture(GL_TEXTURE0);
  GL.glBindTexture(GL_TEXTURE_2D, txd);
  GL.glTexImage2D(
      GL_TEXTURE_2D, 0, color_format, img->width, img->height, 0, color_format, color_type, img->data);
  GL.glGenerateMipmap(GL_TEXTURE_2D);
  GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  GL.glBindTexture(GL_TEXTURE_2D, 0);

  texture.id = txd;
  texture.width = img->width;
  texture.height = img->height;
  texture.format = img->format;
  texture.mipmap = 1;

  return texture;
}

void destroy_texture_g(struct texture_f texture) {
  GL.glDeleteTextures(1, (const GLuint *)&texture.id);
}

struct sprite_f create_sprite_g(const char *texture_name, struct rect_f rect) {
  struct texture_f texture = load_texture_f(texture_name);
  struct sprite_f sprite = create_sprite_from_texture_g(texture, rect);

  return sprite;
}

struct sprite_f create_sprite_from_texture_g(struct texture_f texture, struct rect_f rect) {
  unsigned int VAO, VBO, IBO;

  GL.glGenVertexArrays(1, &VAO);
  GL.glBindVertexArray(VAO);

  struct color_f color = WHITE;

  // Vertices = 3.f Colors = 4.f Texture 2.f
  float vertices[] = {
      //[Vector =====] [============= Color =============] [Texture]
      1.f,  1.f,  0.f, color.r, color.g, color.b, color.a, 1.f, 1.f, // Top Right
      1.f,  -1.f, 0.f, color.r, color.g, color.b, color.a, 1.f, 0.f, // Bottom Right
      -1.f, -1.f, 0.f, color.r, color.g, color.b, color.a, 0.f, 0.f, // Bottom Left
      -1.f, 1.f,  0.f, color.r, color.g, color.b, color.a, 0.f, 1.f, // Top Left
  };

  GL.glGenBuffers(1, &VBO);
  GL.glBindBuffer(GL_ARRAY_BUFFER, VBO);
  GL.glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  GL.glEnableVertexAttribArray(0);
  GL.glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 9 * sizeof(float), NULL);
  GL.glEnableVertexAttribArray(1);
  GL.glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
  GL.glEnableVertexAttribArray(2);
  GL.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(7 * sizeof(float)));

  int index[] = {0, 1, 2, 2, 3, 0};
  GL.glGenBuffers(1, &IBO);
  GL.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
  GL.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

  struct sprite_f sprite = {0};
  sprite.vao = VAO;
  sprite.position = (struct vector3_f){rect.x, rect.y, 0.f};
  sprite.scale = (struct vector3_f){rect.width, rect.height, 1.f};
  sprite.texture = texture;

  return sprite;
}

void destroy_sprite_g(struct sprite_f sprite) {
  GL.glDeleteBuffers(1, &sprite.vao);
  destroy_texture_g(sprite.texture);
}

void draw_sprite_g(struct sprite_f sprite) {
  struct mat4_f m_translate = matrix_init_translation_f(sprite.position.x, sprite.position.y, sprite.position.z);
  struct mat4_f m_rotation =
      matrix_init_rotation_f(sprite.rotation.pitch, sprite.rotation.yaw, sprite.rotation.roll);
  struct mat4_f m_scale = matrix_init_scale_f(sprite.scale.x, sprite.scale.y, sprite.scale.z);
  struct mat4_f m_model = matrix_identity_f();

  m_model = matrix_mult_f(m_rotation, m_scale);
  m_model = matrix_mult_f(m_translate, m_model);

  GL.glUniformMatrix4fv(
      GL.glGetUniformLocation(default_shader, "proj"), 1, GL_FALSE, (const float *)get_projection_matrix_g().e);
  GL.glUniformMatrix4fv(GL.glGetUniformLocation(default_shader, "model"), 1, GL_FALSE, (const float *)m_model.e);

  GL.glUseProgram(default_shader);
  GL.glBindVertexArray(sprite.vao);
  GL.glActiveTexture(GL_TEXTURE0);
  GL.glBindTexture(GL_TEXTURE_2D, sprite.texture.id);
  GL.glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

// Need to Refactor
shader_f load_shader_g(const char *vertex_path, const char *fragment_path) {
  int status;
  const int log_size = 512;
  GLsizei error_buffer_len = 0;
  char error_buffer[1024];
  FILE *file = NULL;
  const char *file_buffer = NULL;
  size_t file_size = 0;

  // Vertex Shader
  file = fopen(vertex_path, "r");
  if (file == NULL) {
    G_LOG(LOG_INFO, "GRAPHIC:Vertex Shader file Not Opening");
  }

  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  file_buffer = (char *)calloc(1, file_size);
  fread((char *)file_buffer, file_size, 1, file);

  GLuint v_shader = GL.glCreateShader(GL_VERTEX_SHADER);
  GL.glShaderSource(v_shader, 1, &file_buffer, 0);
  GL.glCompileShader(v_shader);

  GL.glGetShaderiv(v_shader, GL_COMPILE_STATUS, &status);
  if (!status) {
    GL.glGetShaderInfoLog(v_shader, log_size, &error_buffer_len, error_buffer);
    G_LOG(LOG_INFO, "GRAPHIC:Compiling Vertex Shader:%s", error_buffer);
    GL.glDeleteShader(v_shader);
    return 0;
  }
  free_memory_f((char *)file_buffer);
  fclose(file);

  // Fragment Shader
  file = fopen(fragment_path, "r");

  if (file == NULL) {
    G_LOG(LOG_INFO, "GRAPHIC:Vertex Shader file Not Opening");
  }

  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  file_buffer = (char *)calloc(1, file_size);
  fread((char *)file_buffer, file_size, 1, file);

  GLuint f_shader = GL.glCreateShader(GL_FRAGMENT_SHADER);
  GL.glShaderSource(f_shader, 1, &file_buffer, 0);
  GL.glCompileShader(f_shader);

  GL.glGetShaderiv(f_shader, GL_COMPILE_STATUS, &status);
  if (!status) {
    GL.glGetShaderInfoLog(f_shader, log_size, &error_buffer_len, error_buffer);
    G_LOG(LOG_FATAL, "GRAPHIC:Compiling Fragment Shader:%s", error_buffer);
    GL.glDeleteShader(v_shader);
    GL.glDeleteShader(f_shader);
    return 0;
  }
  free_memory_f((char *)file_buffer);
  fclose(file);

  // Shader Program

  GLuint shader_program = GL.glCreateProgram();
  GL.glAttachShader(shader_program, v_shader);
  GL.glAttachShader(shader_program, f_shader);

  GL.glLinkProgram(shader_program);

  GL.glGetProgramiv(f_shader, GL_LINK_STATUS, &status);
  if (!status) {
    GL.glGetProgramInfoLog(f_shader, log_size, &error_buffer_len, error_buffer);
    G_LOG(LOG_INFO, "GRAPHIC:Linking Shaders:%s", error_buffer);
    GL.glDeleteShader(v_shader);
    GL.glDeleteShader(f_shader);
    GL.glDeleteProgram(shader_program);
    return 0;
  }

  GL.glDeleteShader(v_shader);
  GL.glDeleteShader(f_shader);

  return shader_program;
}
