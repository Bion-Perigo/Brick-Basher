#include "GL/glcorearb.h"
#include "core.h"

#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_memory_f(size_t data_size) {
  return (char *)calloc(1, data_size);
}

char *resized_memory_f(void *data, size_t new_size) {
  return (char *)realloc(data, new_size);
}

char *copy_memory_f(void *destiny, void *source, size_t copy_size) {
  return (char *)memcpy((void *)destiny, (void *)source, copy_size);
}

bool free_memory_f(void *data) {
  if (data == NULL) {
    G_LOG(LOG_INFO, "MEMORY:Free Memory Received NULL");
    return false;
  }
  free((char *)data);

  return true;
}

char *load_buffer_file(const char *file_name, size_t *file_size, size_t extra_size, size_t offset) {
  FILE *file = NULL;
  char *buffer = NULL;
  size_t buffer_size = 0;
  size_t total_size = 0;

  file = fopen(file_name, "rb");

  if (file == NULL) {
    G_LOG(LOG_INFO, "MEMORY:File Not Opened->%s", file_name);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  buffer_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (file_size != NULL) {
    *file_size = buffer_size;
  }

  total_size = buffer_size + extra_size;

  buffer = get_memory_f(total_size);
  if (buffer == NULL) {
    G_LOG(LOG_INFO, "MEMORY:Buffer not valid");
    fclose(file);
    return NULL;
  }

  fread(buffer + offset, buffer_size, 1, file);
  fclose(file);

  return buffer;
}

shader_f load_shader_f(const char *vertex_path, const char *fragment_path) {
  int status;
  const int log_size = 512;
  GLsizei error_buffer_len = 0;
  char error_buffer[error_buffer_len];
  FILE *file = NULL;
  const char *file_buffer = NULL;
  size_t file_size = 0;

  // Vertex Shader
  file = fopen(vertex_path, "r");
  if (file == NULL) {
    G_LOG(LOG_INFO, "MEMORY:Vertex Shader file Not Opening");
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
    G_LOG(LOG_INFO, "MEMORY:Compiling Vertex Shader:%s", error_buffer);
    GL.glDeleteShader(v_shader);
    return 0;
  }
  free_memory_f((char *)file_buffer);
  fclose(file);

  // Fragment Shader
  file = fopen(fragment_path, "r");

  if (file == NULL) {
    G_LOG(LOG_INFO, "MEMORY:Vertex Shader file Not Opening");
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
    G_LOG(LOG_FATAL, "MEMORY:Compiling Fragment Shader:%s", error_buffer);
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
    G_LOG(LOG_INFO, "MEMORY:Linking Shaders:%s", error_buffer);
    GL.glDeleteShader(v_shader);
    GL.glDeleteShader(f_shader);
    GL.glDeleteProgram(shader_program);
    return 0;
  }

  GL.glDeleteShader(v_shader);
  GL.glDeleteShader(f_shader);

  return shader_program;
}
