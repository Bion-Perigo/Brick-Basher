#pragma once
#include "EGL/egl.h"
#include "GL/gl_api.h"
#include "core.h"

#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define CONTEXT_LOG                         " -> File:" STR(__FILE__) " Line:" STR(__LINE__)
#define BUFFER_LOG                          1024
#define STRING_STACK                        30
#define NO_EXPAND(a)                        #a
#define STR(a)                              NO_EXPAND(a)
#define CONCAT(a, b)                        a##b
#define FIND_CONFIG(config_path)            STR(CONFIG_PATH) config_path
#define FIND_ASSET(asset_path)              STR(CONTENT_PATH) asset_path
#define G_LOG(log_level, format, ...)       create_log_p(log_level, CONTEXT_LOG, format, ##__VA_ARGS__);
#define CALL_API(function_ptr, return, ...) (function_ptr != NULL) ? function_ptr(__VA_ARGS__) : return

typedef unsigned int shader_f;

#ifndef M_PI
#define M_PI 3.1415
#endif // M_PI

/*==================== Game Platform ====================*/

// Log =====================
enum log_level_p {
  LOG_INFO,    //
  LOG_SUCCESS, //
  LOG_WARNING, //
  LOG_ERROR,   //
  LOG_FATAL    //
};

void create_log_p(enum log_level_p level, const char *context, const char *format, ...);

// All OpenGL functions // Temp
extern struct GL_API GL;

/*==================== Game Framework ====================*/

enum window_anchor_f {
  SCREEN_LEFT = 0,
  SCREEN_RIGHT = 100,
  SCREEN_TOP = 100,
  SCREEN_BOTTON = 0,
  SCREEN_CENTER = 50,
};

enum camera_mode_f {
  CAMERA_PERSPECTIVE, // Not Implemented
  CAMERA_ORTHOGRAPHIC,
};

// Basic Colors
struct color_f {
  float r;
  float g;
  float b;
  float a;
};

struct image_f {
  char *data;
  int width;
  int height;
  int format;
};

struct texture_f {
  int id;
  int width;
  int height;
  int format;
  int mipmap;
};

// Matrix
struct mat4_f {
  float e[4][4];
};

struct vector2_f {
  float x;
  float y;
};

struct vector3_f {
  float x;
  float y;
  float z;
};

struct rotator_f {
  float pitch;
  float yaw;
  float roll;
};

struct rect_f {
  float x;
  float y;
  float width;
  float height;
};

struct sprite_f {
  unsigned int vao;
  struct texture_f texture;
  struct vector3_f position;
  struct rotator_f rotation;
  struct vector3_f scale;
  struct vector2_f uv;
  struct vector2_f frames;
};

enum buttom_state_f {
  BUTTOM_NONE,
  BUTTOM_HOVERED,
  BUTTON_UNHOVERED,
  BUTTOM_PRESSED,
  BUTTON_RELEASED,
};

struct buttom_f {
  int index;
  struct sprite_f icon;
  enum buttom_state_f state;
  void (*on_hovered)(struct buttom_f *buttom);
  void (*on_unhovered)(struct buttom_f *buttom);
  void (*on_pressed)(struct buttom_f *buttom);
  void (*on_release)(struct buttom_f *buttom);
  void (*on_clicked)(struct buttom_f *buttom);
};

// Make Vectors

#define VEC2(x, y)     \
  (struct vector2_f) { \
    x, y               \
  }

#define VEC3(x, y, z)  \
  (struct vector3_f) { \
    x, y, z            \
  }

#define CLITERAL(type) (type)
//#define CLITERAL(type) type

#define BLANK                \
  CLITERAL(struct color_f) { \
    0.f, 0.f, 0.f, 0.f       \
  }
#define WHITE                \
  CLITERAL(struct color_f) { \
    1.f, 1.f, 1.f, 1.f       \
  }
#define BLACK                \
  CLITERAL(struct color_f) { \
    0.f, 0.f, 0.f, 1.f       \
  }
#define RED                  \
  CLITERAL(struct color_f) { \
    1.f, 0.f, 0.f, 1.f       \
  }
#define GREEN                \
  CLITERAL(struct color_f) { \
    0.f, 1.f, 0.f, 1.f       \
  }
#define BLUE                 \
  CLITERAL(struct color_f) { \
    0.f, 0.f, 1.f, 1.f       \
  }
#define YELLOW               \
  CLITERAL(struct color_f) { \
    1.f, 1.f, 0.f, 1.f       \
  }
#define VIOLET               \
  CLITERAL(struct color_f) { \
    1.f, 0.f, 1.f, 1.f       \
  }
#define CYAN                 \
  CLITERAL(struct color_f) { \
    0.f, 1.f, 1.f, 1.f       \
  }
#define BROWN                \
  CLITERAL(struct color_f) { \
    0.65f, 0.16f, 0.16f, 1.f \
  }
#define GRAY                 \
  CLITERAL(struct color_f) { \
    0.75f, 0.75f, 0.75f, 1.f \
  }
#define SKY_BLUE             \
  CLITERAL(struct color_f) { \
    0.20f, 0.6f, 0.8f, 1.f   \
  }
#define DARK_BROWN           \
  CLITERAL(struct color_f) { \
    0.36f, 0.25f, 0.20f, 1.f \
  }
#define DARK_GRAY            \
  CLITERAL(struct color_f) { \
    0.45f, 0.45f, 0.45f, 1.f \
  }
#define DARK_PURPLE          \
  CLITERAL(struct color_f) { \
    0.53f, 0.12f, 0.47f, 1.f \
  }

// Levels in C
struct level_c_f {
  void (*on_level_start)();
  void (*on_level_update)(float delta_time);
  void (*on_level_draw)(float delta_time);
  void (*on_level_end)();
};

/*==================== Game Platform ====================*/

struct window_p {
  int width;
  int height;
  const char *title;
  int screen;
  bool should_close;
  bool fullscreen;

  NativeDisplayType display;
  NativeWindowType window;
};

struct window_api_p {
  struct window_p *(*on_create_window)(int width, int height, const char *title);
  void (*on_update_window)();
  void (*on_close_window)();
  void (*on_set_window_fullscreen)();
  void (*on_show_cursor)(bool b_show);
};

enum platform_p {
  WINDOWS, //
  LINUX,   //
};

enum keybord_p {
  KEY_APOSTROPHE,           // Key: '
  KEY_COMMA,                // Key: ,
  KEY_MINUS,                // Key: -
  KEY_PERIOD,               // Key: .w
  KEY_SLASH,                // Key: /
  KEY_ZERO,                 // Key: 0
  KEY_ONE,                  // Key: 1
  KEY_TWO,                  // Key: 2
  KEY_THREE,                // Key: 3
  KEY_FOUR,                 // Key: 4
  KEY_FIVE,                 // Key: 5
  KEY_SIX,                  // Key: 6
  KEY_SEVEN,                // Key: 7
  KEY_EIGHT,                // Key: 8
  KEY_NINE,                 // Key: 9
  KEY_SEMICOLON,            // Key: ;
  KEY_EQUAL,                // Key: =
  KEY_A,                    // Key: A | a
  KEY_B,                    // Key: B | b
  KEY_C,                    // Key: C | c
  KEY_D,                    // Key: D | d
  KEY_E,                    // Key: E | e
  KEY_F,                    // Key: F | f
  KEY_G,                    // Key: G | g
  KEY_H,                    // Key: H | h
  KEY_I,                    // Key: I | i
  KEY_J,                    // Key: J | j
  KEY_K,                    // Key: K | k
  KEY_L,                    // Key: L | l
  KEY_M,                    // Key: M | m
  KEY_N,                    // Key: N | n
  KEY_O,                    // Key: O | o
  KEY_P,                    // Key: P | p
  KEY_Q,                    // Key: Q | q
  KEY_R,                    // Key: R | r
  KEY_S,                    // Key: S | s
  KEY_T,                    // Key: T | t
  KEY_U,                    // Key: U | u
  KEY_V,                    // Key: V | v
  KEY_W,                    // Key: W | w
  KEY_X,                    // Key: X | x
  KEY_Y,                    // Key: Y | y
  KEY_Z,                    // Key: Z | z
  KEY_LEFT_BRACKET,         // Key: [
  KEY_BACKSLASH,            // Key: '\'
  KEY_RIGHT_BRACKET,        // Key: ]
  KEY_GRAVE,                // Key: `
  KEY_SPACE,                // Key: Space
  KEY_ESCAPE,               // Key: Esc
  KEY_ENTER,                // Key: Enter
  KEY_TAB,                  // Key: Tab
  KEY_BACKSPACE,            // Key: Backspace
  KEY_INSERT,               // Key: Ins
  KEY_DELETE,               // Key: Del
  KEY_RIGHT,                // Key: Cursor right
  KEY_LEFT,                 // Key: Cursor left
  KEY_DOWN,                 // Key: Cursor down
  KEY_UP,                   // Key: Cursor up
  KEY_PAGE_UP,              // Key: Page up
  KEY_PAGE_DOWN,            // Key: Page down
  KEY_HOME,                 // Key: Home
  KEY_END,                  // Key: End
  KEY_CAPS_LOCK,            // Key: Caps lock
  KEY_SCROLL_LOCK,          // Key: Scroll down
  KEY_NUM_LOCK,             // Key: Num lock
  KEY_PRINT_SCREEN,         // Key: Print screen
  KEY_PAUSE,                // Key: Pause
  KEY_F1,                   // Key: F1
  KEY_F2,                   // Key: F2
  KEY_F3,                   // Key: F3
  KEY_F4,                   // Key: F4
  KEY_F5,                   // Key: F5
  KEY_F6,                   // Key: F6
  KEY_F7,                   // Key: F7
  KEY_F8,                   // Key: F8
  KEY_F9,                   // Key: F9
  KEY_F10,                  // Key: F10
  KEY_F11,                  // Key: F11
  KEY_F12,                  // Key: F12
  KEY_LEFT_SHIFT,           // Key: Shift left
  KEY_LEFT_CONTROL,         // Key: Control left
  KEY_LEFT_ALT,             // Key: Alt left
  KEY_LEFT_SUPER,           // Key: Super left
  KEY_RIGHT_SHIFT,          // Key: Shift right
  KEY_RIGHT_CONTROL,        // Key: Control right
  KEY_RIGHT_ALT,            // Key: Alt right
  KEY_RIGHT_SUPER,          // Key: Super right
  KEY_KB_MENU,              // Key: KB menu
  KEY_KP_0,                 // Key: Keypad 0
  KEY_KP_1,                 // Key: Keypad 1
  KEY_KP_2,                 // Key: Keypad 2
  KEY_KP_3,                 // Key: Keypad 3
  KEY_KP_4,                 // Key: Keypad 4
  KEY_KP_5,                 // Key: Keypad 5
  KEY_KP_6,                 // Key: Keypad 6
  KEY_KP_7,                 // Key: Keypad 7
  KEY_KP_8,                 // Key: Keypad 8
  KEY_KP_9,                 // Key: Keypad 9
  KEY_KP_DECIMAL,           // Key: Keypad .
  KEY_KP_DIVIDE,            // Key: Keypad /
  KEY_KP_MULTIPLY,          // Key: Keypad *
  KEY_KP_SUBTRACT,          // Key: Keypad -
  KEY_KP_ADD,               // Key: Keypad +
  KEY_KP_ENTER,             // Key: Keypad Enter
  KEY_KP_EQUAL,             // Key: Keypad =
  KEY_MOUSE_LEFT,           // Mouse Left
  KEY_MOUSE_MIDDLE,         // Mouse Middle
  KEY_MOUSE_RIGHT,          // Mouse Right
  KEY_MOUSE_WHEEL_FORWARD,  // Mouse Forward
  KEY_MOUSE_WHEEL_BACKWARD, // Mouse Backward
  KEY_MAX
};

// Used only by platforms.
#define MOUSE_LEFT_CODE     1
#define MOUSE_MIDDLE_CODE   2
#define MOUSE_RIGHT_CODE    3
#define MOUSE_FORWARD_CODE  4
#define MOUSE_BACKWARD_CODE 5

/*====================* Game Functions *====================*/

/*==================== Game Platform ====================*/

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Window =====================
struct window_p *init_window_p(int width, int height, const char *title);
void close_window_p();
void update_window_p();
bool window_should_close_p();
void set_window_fullscreen_p();
struct window_p *get_window_p();
void get_window_size_p(int *width, int *height);
int get_window_width_p();
int get_window_height_p();
void set_show_cursor_p(bool b_show);
bool get_show_cursor_p();
bool init_opengl_p(int major, int minor, int color_bits, int depth_bits);
void begin_frame_p();
void end_frame_p();

// Functionalities ====================
void quit_game_p();
float get_time_p();
void wait_time_p(double time);

/*==================== Game Graphic ====================*/
void init_graphic_g();
void update_graphic_g();
void clear_background_g(struct color_f color);
int get_camera_mode_g();
void set_camera_mode_g(enum camera_mode_f mode);
struct mat4_f get_projection_matrix_g();
struct mat4_f get_view_matrix_g();
struct texture_f create_texture_g(struct image_f *image);
void destroy_texture_g(struct texture_f texture);
struct sprite_f create_sprite_g(const char *texture_name, struct rect_f rect);
struct sprite_f create_sprite_from_texture_g(struct texture_f texture, struct rect_f rect);
void destroy_sprite_g(struct sprite_f sprite);
void draw_sprite_g(struct sprite_f sprite);
void create_flip_book_f(const char *file_name, int frames, int colluns, int lines);
shader_f load_shader_g(const char *vertex_path, const char *fragment_path);

// Library =====================
void *load_library_p(const char *library_name);
bool free_library_p(void *library);
void *get_function_p(void *library, const char *name);
void get_functions_p(void *library, void *api, const char **names);

/*==================== Game Framework ====================*/

// Timer Manager
void update_timer_f();
void begin_time_f();
void end_time_f();
void set_target_fps_f(int max_fps);
int get_fps_f();
float get_frametime_f();

// Collision Manager =====================
bool check_collision_sprite_f(struct sprite_f a, struct sprite_f b);
bool check_collision_sprite_pointer_f(struct sprite_f a, struct vector2_f v);

// Input Manager =====================
bool is_key_pressed_f(int key_code);
bool is_key_released_f(int key_code);
bool is_key_repeat_f(int key_code);
void get_mouse_position_f(int *x, int *y);
int get_mouse_x_f();
int get_mouse_y_f();
struct vector2_f get_mouse_screen_position_f();

// Memory Manager =====================
char *get_memory_f(size_t data_size);
char *resized_memory_f(void *data, size_t new_size);
char *copy_memory_f(void *destiny, void *source, size_t copy_size);
bool free_memory_f(void *data);
char *load_buffer_file(const char *file_name, size_t *file_size, size_t extra_size, size_t offset);

// Asset Manager
struct image_f *load_image_default_f();
struct image_f *load_image_f(const char *file_name);
struct texture_f load_texture_f(const char *file_name);

// Math
float map_range_f(float in_a, float in_b, float out_a, float out_b, float value);
struct vector3_f vector3_add_scale(struct vector3_f v, float scale);
struct vector3_f vector3_sub_scale(struct vector3_f v, float scale);

// UserInterface
struct buttom_f create_buttom_f(struct rect_f rect);
void update_buttom_f(struct buttom_f *buttom);

// Utility
const char *get_file_extension_f(const char *file_name);

// Game
void open_level_c_f(struct level_c_f level);

// Matrix =====================
struct mat4_f matrix_identity_f();
struct mat4_f matrix_mult_f(struct mat4_f a, struct mat4_f b);
struct mat4_f matrix_inverse_f(struct mat4_f m);
struct mat4_f matrix_init_perspective_f(float fov, float aspect, float z_near, float z_far);
struct mat4_f matrix_init_ortho_f(float left, float right, float top, float bottom, float z_near, float z_far);
struct mat4_f matrix_init_translation_f(float x, float y, float z);
struct mat4_f matrix_init_rotation_f(float x, float y, float z);
struct mat4_f matrix_init_scale_f(float x, float y, float z);

#ifdef __cplusplus
}

#endif // __cplusplus
