#include "core.h"

#include <stdio.h>
#include <string.h>

#pragma pack(1)
struct bitmap_h {
  unsigned short Type;
  unsigned int File_Size;
  unsigned short reserved1;
  unsigned short reserved2;
  unsigned int offBits;
  unsigned int size;
  unsigned int width;
  unsigned int height;
  unsigned short planes;
  unsigned short bitCount;
  unsigned int compression;
  unsigned int sizeImage;
  unsigned int xPelsPerMeter;
  unsigned int yPelsPerMeter;
  unsigned int clrUsed;
  unsigned int clrImportant;
};
#pragma pack(8) // All

//__attribute__((packed)); // GCC and Clang

struct image_f *load_bitmap(const char *file_name) {
  const int BITMAP_TYPE = 0x4D42;
  const int BITMAP_COMPRESSION = 3;

  size_t image_size = sizeof(struct image_f);
  struct bitmap_h *bitmap = NULL;
  struct image_f *image = NULL;
  char *buffer = load_buffer_file(file_name, NULL, image_size, image_size);

  image = (struct image_f *)buffer;
  bitmap = (struct bitmap_h *)(buffer + image_size);

  if (buffer == NULL) {
    return NULL;
  }
  if (bitmap->Type != BITMAP_TYPE) {
    G_LOG(LOG_INFO, "ASSET:Image Is Not Actually a Bitmap.");
    free_memory_f(buffer);
    return NULL;
  }
  if (bitmap->compression != BITMAP_COMPRESSION) {
    G_LOG(LOG_INFO, "ASSET:Bitmap Compression Not Supported:%d", bitmap->compression);
    free_memory_f(buffer);
    return NULL;
  }

  image->width = bitmap->width;
  image->height = bitmap->height;
  image->format = bitmap->bitCount;
  image->data = bitmap->offBits + (char *)bitmap;

  if (image->format == 24 || image->format == 32) {
    unsigned int *mask_ptr = (unsigned int *)(sizeof(struct bitmap_h) + (char *)bitmap);
    unsigned int r_mask = *mask_ptr++;
    unsigned int g_mask = *mask_ptr++;
    unsigned int b_mask = *mask_ptr++;
    unsigned int a_mask = ~(r_mask | g_mask | b_mask);

    unsigned int r_shift = (r_mask == 0xFF000000) ? 24 : (r_mask == 0xFF0000) ? 16 : (r_mask == 0xFF00) ? 8 : 0;
    unsigned int g_shift = (g_mask == 0xFF000000) ? 24 : (g_mask == 0xFF0000) ? 16 : (g_mask == 0xFF00) ? 8 : 0;
    unsigned int b_shift = (b_mask == 0xFF000000) ? 24 : (b_mask == 0xFF0000) ? 16 : (b_mask == 0xFF00) ? 8 : 0;
    unsigned int a_shift = (a_mask == 0xFF000000) ? 24 : (a_mask == 0xFF0000) ? 16 : (a_mask == 0xFF00) ? 8 : 0;

    const int num_pixel = image->width * image->height;
    const int num_bytes = image->format / 8;
    for (int i = 0; i < num_pixel; i++) {
      unsigned int *pixel_ptr = (unsigned int *)(i * num_bytes + image->data);
      unsigned int pixel = *pixel_ptr;

      unsigned int r = (pixel & r_mask) >> r_shift;
      unsigned int g = (pixel & g_mask) >> g_shift;
      unsigned int b = (pixel & b_mask) >> b_shift;
      unsigned int a = (pixel & a_mask) >> a_shift;

      unsigned color = a << 24 | b << 16 | g << 8 | r;

      *pixel_ptr = color;
    }

  } else if (image->format != 16) {
    G_LOG(LOG_INFO, "ASSET:Bitmap Format Not Suported:%d", image->format);

    return NULL;
  }

  return image;
}

struct image_f *load_image_default_f(int width, int height) {
  static struct image_f default_image = {0};
  if (default_image.data != NULL) {
    return &default_image;
  }

  const int tex_width = 512;
  const int tex_height = 512;
  const int tex_format = 24;
  const int square_count = 8;
  const int square_size = tex_width / square_count;

  unsigned char *data = (unsigned char *)get_memory_f(tex_width * tex_height * tex_format);
  unsigned char *pixel = data;

  for (int i = 0; i < tex_width; i++) {
    for (int j = 0; j < tex_width; j++) {
      int x = i / square_size;
      int y = j / square_size;
      int square_number = x * square_count + y;

      unsigned char color;
      bool is_odd = (square_number & 1);
      if (x & 1) {
        color = (is_odd) ? 0xAA : 0x55;
      } else {
        color = (is_odd) ? 0x55 : 0xAA;
      }
      *pixel++ = color;
      *pixel++ = color;
      *pixel++ = color;
    }
  }

  default_image.width = tex_width;
  default_image.height = tex_height;
  default_image.format = tex_format;
  default_image.data = (char *)data;

  G_LOG(LOG_INFO, "ASSET:Default Image Loaded!");
  return &default_image;
}

struct image_f *load_image_f(const char *file_name) {
  struct image_f *image = NULL;
  if (file_name == NULL) {
    G_LOG(LOG_INFO, "ASSET:Image Not Loaded File Name Invalid!");
    return NULL;
  }

  const char *type = get_file_extencion(file_name);

  if (strcmp(type, ".bmp") || strcmp(type, ".BMP")) {
    image = load_bitmap(file_name);
  } else {
    G_LOG(LOG_INFO, "ASSET:Image Not Loaded, Type Not Supported:%s", type);
  }

  if (image != NULL) {
    G_LOG(LOG_INFO, "ASSET:Image Loaded->%s", file_name);
    return image;
  }
  G_LOG(LOG_INFO, "ASSET:Image Not Loaded->%s", file_name);
  return load_image_default_f(1, 1);
}
