#include "core.h"

#define BUTTOM_CALLBACK(function_ptr) \
  if (function_ptr != NULL) {         \
    function_ptr(buttom);             \
  }

struct buttom_f create_buttom_f(struct rect_f rect) {
  struct buttom_f buttom = {0};
  return buttom;
}

void update_buttom_f(struct buttom_f *buttom) {
  if (!get_show_cursor_p()) {
    return;
  }

  if (check_collision_sprite_pointer_f(buttom->icon, get_mouse_screen_position_f())) {
    if (buttom->state != BUTTOM_HOVERED) {
      BUTTOM_CALLBACK(buttom->on_hovered);
      buttom->state = BUTTOM_HOVERED;
    }
    if (is_key_pressed_f(KEY_MOUSE_LEFT) && buttom->state != BUTTOM_PRESSED) {
      BUTTOM_CALLBACK(buttom->on_pressed);
    } else if (is_key_repeat_f(KEY_MOUSE_LEFT)) {
      BUTTOM_CALLBACK(buttom->on_clicked);
    }
    if (is_key_released_f(KEY_MOUSE_LEFT) && buttom->state != BUTTON_RELEASED) {
      BUTTOM_CALLBACK(buttom->on_release);
    }
  } else if (buttom->state != BUTTOM_NONE) {
    BUTTOM_CALLBACK(buttom->on_unhovered);
    buttom->state = BUTTOM_NONE;
  }
}
