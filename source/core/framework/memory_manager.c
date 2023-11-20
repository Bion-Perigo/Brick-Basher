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


