#include "core.h"
#include "level_1.h"

struct level_f current_level;

void game_start(){
  current_level = load_level_1();
  current_level.on_level_begin();
}

void game_update(float delta_time){
  current_level.on_level_update(delta_time);
}

void game_draw(float delta_time){
  current_level.on_level_draw(delta_time);
}

void game_stop(){
  current_level.on_level_end();
}
