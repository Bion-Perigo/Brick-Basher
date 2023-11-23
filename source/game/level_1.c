#include "core.h"

#include <stdbool.h>

#define BRICK_COUNT 33

extern struct level_f load_main_menu();

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
  bool ball_stopped;
  int player_life;
} * this;

struct level_f load_level_1() {
  struct level_f level = {0};
  level.on_level_start = &level_start;
  level.on_level_update = &level_update;
  level.on_level_draw = &level_draw;
  level.on_level_end = &level_end;
  this = (struct level_1 *)get_memory_f(sizeof(struct level_1));
  return level;
}

void level_start() {
  set_show_cursor_p(false);

  this->ball_speed = (struct vector2_f){50.f, 50.f};
  this->max_speed = 100.f;
  this->normal_speed = 50.f;
  this->gravity = 0.01f;
  this->ball_stopped = true;
  this->right_value = 1.f;
  this->player_life = 3;

  struct rect_f level_rect = {SCREEN_LEFT + 50, SCREEN_BOTTON + 50, 50, 50};
  this->level = create_sprite_g(FIND_ASSET("texture/actors/level_1_background.bmp"), level_rect);
  this->level.position.z = -1; // Set Backgroud Position

  struct rect_f player_rect = {50.f, 2.f, 6.f, 3.f};
  this->player = create_sprite_g(FIND_ASSET("texture/actors/player.bmp"), player_rect);

  struct rect_f life_rect = {SCREEN_CENTER, SCREEN_CENTER, 2, 2};
  this->life = create_sprite_g(FIND_ASSET("texture/actors/life.bmp"), life_rect);
  this->life.frames.x = 4;
  this->life.uv.x = this->life.frames.x - 1;

  struct rect_f ball_rect = {0.f, 0.f, 2.f, 2.f};
  this->ball = create_sprite_g(FIND_ASSET("texture/actors/ball.bmp"), ball_rect);

  struct rect_f brick_rect[BRICK_COUNT] = {0};
  this->brick_texture = load_texture_f(FIND_ASSET("texture/actors/brick.bmp"));

  struct vector2_f brick_size = {4, 2};
  float min_distance = 5.f + brick_size.x;
  int per_line = 11;
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
    this->bricks[i] = create_sprite_from_texture_g(this->brick_texture, brick_rect[i]);
  }
}

void level_update(float delta_time) {

  if (is_key_pressed_f(KEY_ESCAPE)) {
    open_level_f(load_main_menu());
  }
  if (is_key_pressed_f(KEY_TAB)) {
    set_window_fullscreen_p();
  }

  if (is_key_repeat_f(KEY_D)) {
    this->right_value = 1;
    this->player.position.x += 100.f * this->right_value * get_frametime_f();
  }
  if (is_key_repeat_f(KEY_A)) {
    this->right_value = -1;
    this->player.position.x += 100.f * this->right_value * get_frametime_f();
  }
  if (is_key_pressed_f(KEY_SPACE) && this->ball_stopped) {
    this->ball_stopped = false;
    this->ball_speed.x = this->max_speed * this->right_value;
    this->ball_speed.y = this->max_speed;
  }

  for (int i = 0; i < BRICK_COUNT; i++) {
    if (check_collision_sprite_f(this->ball, this->bricks[i])) {
      this->bricks[i].position.y = SCREEN_TOP + 100;
    }
  }

  if (check_collision_sprite_f(this->player, this->ball)) {
    this->ball_speed.x = this->max_speed * this->right_value;
    this->ball_speed.y = this->max_speed;
  }

  if (this->player.position.x - this->player.scale.x < SCREEN_LEFT) {
    this->player.position.x = SCREEN_LEFT + this->player.scale.x;
  } else if (this->player.position.x + this->player.scale.x > SCREEN_RIGHT) {
    this->player.position.x = SCREEN_RIGHT - this->player.scale.x;
  }

  if (this->player.position.y - this->player.scale.y < SCREEN_BOTTON) {
    this->player.position.y = SCREEN_BOTTON + this->player.scale.y;
  } else if (this->player.position.y + this->player.scale.y > SCREEN_TOP) {
    this->player.position.y = SCREEN_TOP - this->player.scale.y;
  }

  if (this->ball.position.x < SCREEN_LEFT) {
    this->ball_speed.x = this->normal_speed * 1;
  } else if (this->ball.position.x + this->ball.scale.x > SCREEN_RIGHT) {
    this->ball_speed.x = this->normal_speed * -1;
  }
  if (this->ball.position.y < SCREEN_BOTTON) {
    this->ball_stopped = true;

    if (this->player_life == 0) {
      open_level_f(load_main_menu());
    }
    this->life.uv.x -= 1;
    this->player_life--;

  } else if (this->ball.position.y + this->ball.scale.y > SCREEN_TOP) {
    this->ball_speed.y = this->normal_speed * -1;
  }

  if (is_key_pressed_f(KEY_BACKSPACE)) {
    destroy_texture_g(this->player.texture);
  }

  this->life.position = this->player.position;

  if (this->ball_stopped) {
    this->ball.position.x = this->player.position.x;
    this->ball.position.y = this->player.position.y + this->player.scale.y + this->ball.scale.y;
  } else {
    this->ball.position.x += this->ball_speed.x * get_frametime_f();
    this->ball.position.y += this->ball_speed.y * get_frametime_f();
    this->ball.position.y -= this->gravity;
    this->ball.rotation.roll += 10.f * this->right_value;
  }
}

void level_draw(float delta_time) {
  draw_sprite_g(this->life);
  draw_sprite_g(this->level);
  draw_sprite_g(this->player);
  draw_sprite_g(this->ball);

  for (int i = 0; i < BRICK_COUNT; i++) {
    draw_sprite_g(this->bricks[i]);
  }
}

void level_end() {
  destroy_sprite_g(this->level);
  destroy_sprite_g(this->player);
  destroy_sprite_g(this->life);
  destroy_sprite_g(this->ball);
  for (int i = 0; i < BRICK_COUNT; i++) {
    destroy_sprite_g(this->bricks[i]);
  }
  free_memory_f(this);
}
