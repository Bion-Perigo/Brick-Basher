#include "core.h"

#include <stdbool.h>

extern struct level_c_f load_main_menu();
extern struct level_c_f load_level_1();

static struct level_c_f current_level;
static bool is_valid_level = false;

void open_level_c_f(struct level_c_f level) {
  if (is_valid_level) {
    current_level.on_level_end();
  }
  current_level = level;
  current_level.on_level_start();
  is_valid_level = true;
}

void game_start() {
  open_level_c_f(load_main_menu());
}

void game_update(float delta_time) {
  current_level.on_level_update(delta_time);
}

void game_draw(float delta_time) {
  current_level.on_level_draw(delta_time);
}

void game_stop() {
  current_level.on_level_end();
}
