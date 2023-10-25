#include "core.h"

#include <stdbool.h>

/*==================== Variables ====================*/

char current_keys[KEY_MAX];
char previous_keys[KEY_MAX];

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
