#include "core.h"

#define BRICK_COUNT 34

// Declaration
void level_1_begin();
void level_1_update(float delta_time);
void level_1_draw(float delta_time);
void level_1_end();

struct level_1 {
  struct sprite_f level;
  struct sprite_f player;
  struct sprite_f ball;
  struct sprite_f bricks[BRICK_COUNT];
  struct texture_f brick_texture;
  struct vector2_f ball_speed;
  float normal_speed;
  float max_speed;
  float gravity;
  float right_value;
  bool ball_stopped;

} * level_1;

struct level_f load_level_1() {
  struct level_f level = {0};
  level.on_level_begin = &level_1_begin;
  level.on_level_update = &level_1_update;
  level.on_level_draw = &level_1_draw;
  level.on_level_end = &level_1_end;
  level_1 = (struct level_1 *)get_memory_f(sizeof(struct level_1));
  return level;
}

void level_1_begin() {
  level_1->ball_speed = (struct vector2_f){50.f, 50.f};
  level_1->max_speed = 100.f;
  level_1->normal_speed = 50.f;
  level_1->gravity = 0.01f;
  level_1->ball_stopped = true;
  level_1->right_value = 1.f;

  struct rect_f level_rect = {SCREEN_LEFT + 50, SCREEN_BOTTON + 50, 50, 50};
  level_1->level = create_sprite_g(FIND_ASSET("texture/actors/level_2_background.bmp"), level_rect);
  level_1->level.position.z = -1; // Set Backgroud Position

  struct rect_f player_rect = {50.f, 2.f, 6.f, 3.f};
  level_1->player = create_sprite_g(FIND_ASSET("texture/actors/player.bmp"), player_rect);

  struct rect_f ball_rect = {0.f, 0.f, 2.f, 2.f};
  level_1->ball = create_sprite_g(FIND_ASSET("texture/actors/ball.bmp"), ball_rect);

  struct rect_f brick_rect[BRICK_COUNT] = {0};
  level_1->brick_texture = load_texture_f(FIND_ASSET("texture/actors/brick.bmp"));

  struct vector2_f brick_size = {4, 2};
  float min_distance = 10.f;
  int per_line = 9;
  int colluns = 0;
  int lines = 1;
  for (int i = 0; i < BRICK_COUNT; i++) {
    float pos_x = (colluns == 0) ? min_distance : 0 + SCREEN_LEFT + colluns * min_distance + brick_size.x;
    float pos_y = SCREEN_TOP - lines * 6;
    colluns++;
    if (colluns == per_line) {
      colluns = 0;
      lines++;
    }
    brick_rect[i] = (struct rect_f){pos_x, pos_y, brick_size.x, brick_size.y};
    level_1->bricks[i] = (struct sprite_f)create_sprite_from_texture_g(level_1->brick_texture, brick_rect[i]);
  }
}

void level_1_update(float delta_time) {

  if (is_key_pressed_f(KEY_ESCAPE)) {
    quit_game_p();
  }
  if (is_key_pressed_f(KEY_TAB)) {
    set_window_fullscreen_p();
  }

  if (is_key_repeat_f(KEY_D)) {
    level_1->right_value = 1;
    level_1->player.position.x += 100.f * level_1->right_value * get_frametime_f();
  }
  if (is_key_repeat_f(KEY_A)) {
    level_1->right_value = -1;
    level_1->player.position.x += 100.f * level_1->right_value * get_frametime_f();
  }
  if (is_key_pressed_f(KEY_SPACE) && level_1->ball_stopped) {
    level_1->ball_stopped = false;
    level_1->ball_speed.x = level_1->max_speed * level_1->right_value;
    level_1->ball_speed.y = level_1->max_speed;
  }
  // G_LOG(LOG_INFO, "Player Position X:%f, Y:%f, Z:%f", player.position.x, player.position.y, player.position.z);
  for (int i = 0; i < BRICK_COUNT; i++) {
    if (check_collision_sprite_f(level_1->ball, level_1->bricks[i])) {
      level_1->bricks[i].position.y = SCREEN_TOP + 100;
    }
  }

  if (check_collision_sprite_f(level_1->player, level_1->ball)) {
    level_1->ball_speed.x = level_1->max_speed * level_1->right_value;
    level_1->ball_speed.y = level_1->max_speed;
  }

  if (level_1->player.position.x - level_1->player.scale.x < SCREEN_LEFT) {
    level_1->player.position.x = SCREEN_LEFT + level_1->player.scale.x;
  } else if (level_1->player.position.x + level_1->player.scale.x > SCREEN_RIGHT) {
    level_1->player.position.x = SCREEN_RIGHT - level_1->player.scale.x;
  }

  if (level_1->player.position.y - level_1->player.scale.y < SCREEN_BOTTON) {
    level_1->player.position.y = SCREEN_BOTTON + level_1->player.scale.y;
  } else if (level_1->player.position.y + level_1->player.scale.y > SCREEN_TOP) {
    level_1->player.position.y = SCREEN_TOP - level_1->player.scale.y;
  }

  if (level_1->ball.position.x < SCREEN_LEFT) {
    level_1->ball_speed.x = level_1->normal_speed * 1;
  } else if (level_1->ball.position.x + level_1->ball.scale.x > SCREEN_RIGHT) {
    level_1->ball_speed.x = level_1->normal_speed * -1;
  }
  if (level_1->ball.position.y < SCREEN_BOTTON) {
    level_1->ball_stopped = true;
  } else if (level_1->ball.position.y + level_1->ball.scale.y > SCREEN_TOP) {
    level_1->ball_speed.y = level_1->normal_speed * -1;
  }

  if (is_key_pressed_f(KEY_BACKSPACE)) {
    destroy_texture_g(level_1->player.texture);
  }

  if (level_1->ball_stopped) {
    level_1->ball.position.x = level_1->player.position.x;
    level_1->ball.position.y = level_1->player.position.y + level_1->player.scale.y + level_1->ball.scale.y;
  } else {
    level_1->ball.position.x += level_1->ball_speed.x * get_frametime_f();
    level_1->ball.position.y += level_1->ball_speed.y * get_frametime_f();
    level_1->ball.position.y -= level_1->gravity;
    level_1->ball.rotation.roll += 10.f * level_1->right_value;
  }
}

void level_1_draw(float delta_time) {
  draw_sprite_g(level_1->level);
  draw_sprite_g(level_1->player);
  draw_sprite_g(level_1->ball);

  for (int i = 0; i < BRICK_COUNT; i++) {
    draw_sprite_g(level_1->bricks[i]);
  }
}

void level_1_end() {
  destroy_sprite_g(level_1->level);
  destroy_sprite_g(level_1->player);
  destroy_sprite_g(level_1->ball);
  for (int i = 0; i < BRICK_COUNT; i++) {
    destroy_sprite_g(level_1->bricks[i]);
  }
  free_memory_f(level_1);
}
