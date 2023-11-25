#include "core.h"

#include <stdbool.h>

#define BRICK_COUNT 35

extern struct level_c_f load_main_menu();

// Declaration
static void level_start();
static void level_update(float delta_time);
static void level_draw(float delta_time);
static void level_end();

static struct level_1 {
  struct sprite_f level;
  struct sprite_f player;
  struct sprite_f life;
  struct sprite_f ball;
  struct sprite_f bricks[BRICK_COUNT];
  struct texture_f brick_texture;
  struct vector2_f ball_speed;
  float normal_speed;
  float max_speed;
  float gravity;
  float right_value;
  float player_speed;
  bool ball_stopped;
  int player_life;
  double time;
} * level;

struct level_c_f load_level_1() {
  struct level_c_f lv = {0};
  lv.on_level_start = &level_start;
  lv.on_level_update = &level_update;
  lv.on_level_draw = &level_draw;
  lv.on_level_end = &level_end;
  level = (struct level_1 *)get_memory_f(sizeof(struct level_1));
  return lv;
}

void level_start() {
  set_show_cursor_p(false);

  level->ball_speed = (struct vector2_f){50.f, 160.f};
  level->max_speed = 55.f;
  level->normal_speed = 50.f;
  level->gravity = 0.2f;
  level->ball_stopped = true;
  level->right_value = 1.f;
  level->player_life = 3;
  level->player_speed = 100.f;

  struct rect_f level_rect = {SCREEN_LEFT + 50, SCREEN_BOTTON + 50, 50, 50};
  level->level = create_sprite_g(FIND_ASSET("texture/actors/level_1_background.bmp"), level_rect);
  level->level.position.z = -1; // Set Backgroud Position

  struct rect_f player_rect = {50.f, 2.f, 6.f, 3.f};
  level->player = create_sprite_g(FIND_ASSET("texture/actors/player.bmp"), player_rect);

  struct rect_f life_rect = {SCREEN_CENTER, SCREEN_CENTER, 2, 2};
  level->life = create_sprite_g(FIND_ASSET("texture/actors/life.bmp"), life_rect);
  level->life.frames.x = 4;
  level->life.uv.x = level->life.frames.x - 1;

  struct rect_f ball_rect = {0.f, 0.f, 2.f, 2.f};
  level->ball = create_sprite_g(FIND_ASSET("texture/actors/ball.bmp"), ball_rect);

  struct rect_f brick_rect[BRICK_COUNT] = {0};
  level->brick_texture = load_texture_f(FIND_ASSET("texture/actors/brick.bmp"));

  struct vector2_f brick_size = {5, 2};
  float min_distance = 15.f + brick_size.x;
  int per_line = 5;
  int colluns = 0;
  int lines = 1;
  for (int i = 0; i < BRICK_COUNT; i++) {
    float pos_x = (colluns == 0) ? min_distance : 0 + SCREEN_LEFT + colluns * min_distance;
    float pos_y = SCREEN_TOP - lines * 6;
    colluns++;
    if (colluns == per_line) {
      colluns = 0;
      lines++;
    }
    brick_rect[i] = (struct rect_f){pos_x, pos_y, brick_size.x, brick_size.y};
    level->bricks[i] = create_sprite_from_texture_g(level->brick_texture, brick_rect[i]);
  }
}
bool valid = true;
void level_update(float delta_time) {
  if (is_key_pressed_f(KEY_ESCAPE)) {
    open_level_c_f(load_main_menu());
  }
  if (is_key_pressed_f(KEY_TAB)) {
    set_window_fullscreen_p();
  }

  if (is_key_repeat_f(KEY_D)) {
    level->right_value = 1 * level->player_speed * delta_time;
    level->player.position.x += level->right_value;
  }
  if (is_key_repeat_f(KEY_A)) {
    level->right_value = -1 * level->player_speed * delta_time;
    level->player.position.x += level->right_value;
  }
  if (is_key_pressed_f(KEY_SPACE) && level->ball_stopped) {
    level->ball_stopped = false;
    level->ball_speed.x = level->max_speed * level->right_value;
    level->ball_speed.y = level->max_speed;
    valid = true;
    level->time = get_time_p();
  }

  for (int i = 0; i < BRICK_COUNT; i++) {
    if (check_collision_sprite_f(level->ball, level->bricks[i])) {
      level->bricks[i].position.y = SCREEN_TOP + 100;
    }
  }

  if (check_collision_sprite_f(level->player, level->ball)) {
    level->ball_speed.x = level->max_speed * level->right_value;
    level->ball_speed.y = level->max_speed;
  }

  if (level->player.position.x - level->player.scale.x < SCREEN_LEFT) {
    level->player.position.x = SCREEN_LEFT + level->player.scale.x;
  } else if (level->player.position.x + level->player.scale.x > SCREEN_RIGHT) {
    level->player.position.x = SCREEN_RIGHT - level->player.scale.x;

    if (valid) {
      float end = get_time_p();
      double time_elapsed = end - level->time;
      G_LOG(LOG_INFO, "Elapsed Time:%lf", time_elapsed);
      valid = false;
    }
  }

  if (level->player.position.y - level->player.scale.y < SCREEN_BOTTON) {
    level->player.position.y = SCREEN_BOTTON + level->player.scale.y;
  } else if (level->player.position.y + level->player.scale.y > SCREEN_TOP) {
    level->player.position.y = SCREEN_TOP - level->player.scale.y;
  }

  if (level->ball.position.x < SCREEN_LEFT) {
    level->ball_speed.x = level->normal_speed * 1;
  } else if (level->ball.position.x + level->ball.scale.x > SCREEN_RIGHT) {
    level->ball_speed.x = level->normal_speed * -1;
  }
  if (level->ball.position.y < SCREEN_BOTTON) {
    level->ball_stopped = true;

    if (level->player_life == 0) {
      open_level_c_f(load_main_menu());
    }
    level->life.uv.x -= 1;
    level->player_life--;

  } else if (level->ball.position.y + level->ball.scale.y > SCREEN_TOP) {
    level->ball_speed.y = level->normal_speed * -1;
  }

  if (is_key_pressed_f(KEY_BACKSPACE)) {
    destroy_texture_g(level->player.texture);
  }

  level->life.position = level->player.position;

  if (level->ball_stopped) {
    level->ball.position.x = level->player.position.x;
    level->ball.position.y = level->player.position.y + level->player.scale.y + level->ball.scale.y;
  } else {
    level->ball.position.x += level->ball_speed.x * delta_time;
    level->ball.position.y += level->ball_speed.y * delta_time;
    level->ball.position.y -= level->gravity;
    level->ball.rotation.roll += 10.f * level->right_value;
  }
}

void level_draw(float delta_time) {
  draw_sprite_g(level->life);
  draw_sprite_g(level->level);
  draw_sprite_g(level->player);
  draw_sprite_g(level->ball);

  for (int i = 0; i < BRICK_COUNT; i++) {
    draw_sprite_g(level->bricks[i]);
  }
}

void level_end() {
  destroy_sprite_g(level->level);
  destroy_sprite_g(level->player);
  destroy_sprite_g(level->life);
  destroy_sprite_g(level->ball);
  for (int i = 0; i < BRICK_COUNT; i++) {
    destroy_sprite_g(level->bricks[i]);
  }
  free_memory_f(level);
  G_LOG(LOG_WARNING, "Clean Level 1");
}
