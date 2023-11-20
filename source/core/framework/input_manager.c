#include "core.h"

#include <stdbool.h>

/*==================== Variables ====================*/

char current_keys[KEY_MAX];
char previous_keys[KEY_MAX];
int mouse_position[2];

/*==================== Definition ====================*/

bool is_key_pressed_f(int key_code) {
  return (current_keys[key_code] == true && previous_keys[key_code] == false);
}

bool is_key_released_f(int key_code) {
  return (current_keys[key_code] == false && previous_keys[key_code] == true);
}

bool is_key_repeat_f(int key_code) {
  return (current_keys[key_code] == true && previous_keys[key_code] == true);
}

void get_mouse_position_f(int *x, int *y) {
  if (x)
    *x = (mouse_position[0] > 0) ? mouse_position[0] : 0;
  if (y)
    *y = (mouse_position[1] > 0) ? mouse_position[1] : 0;
}

int get_mouse_x_f() {
  return (mouse_position[0] > 0) ? mouse_position[0] : 0;
}

int get_mouse_y_f() {
  return (mouse_position[1] > 0) ? mouse_position[1] : 0;
}
