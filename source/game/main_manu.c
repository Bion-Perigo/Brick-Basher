#include "core.h"

extern struct level_f load_level_1();

static struct main_menu {
  struct sprite_f title;
  struct sprite_f credit;
  struct sprite_f background;
  struct buttom_f btn_start_game;
  struct buttom_f btn_fullscreen;
  struct buttom_f btn_quit_game;
} * this;

// Declaration
static void level_start();
static void level_update(float delta_time);
static void level_draw(float delta_time);
static void level_end();

// Buttom
void buttom_on_hovered(struct buttom_f *buttom);
void buttom_on_unhovered(struct buttom_f *buttom);
void buttom_on_start_game(struct buttom_f *buttom);
void buttom_on_set_fullscreen(struct buttom_f *buttom);
void buttom_on_quit_game(struct buttom_f *buttom);
void buttom_on_clickd(struct buttom_f *buttom);

struct level_f load_main_menu() {
  struct level_f lv = {0};
  lv.on_level_start = &level_start;
  lv.on_level_update = &level_update;
  lv.on_level_draw = &level_draw;
  lv.on_level_end = &level_end;
  this = (struct main_menu *)get_memory_f(sizeof(struct main_menu));
  return lv;
}

void level_start() {
  set_show_cursor_p(true);

  struct rect_f bg_rect = {SCREEN_CENTER, SCREEN_CENTER, 50, 50};
  this->background = create_sprite_g(FIND_ASSET("texture/main_menu/background.bmp"), bg_rect);
  this->background.position.z = -1;
  struct rect_f credit_rect = {SCREEN_CENTER, SCREEN_BOTTON + 10, 45, 3};
  this->credit = create_sprite_g(FIND_ASSET("texture/main_menu/credit.bmp"), credit_rect);

  struct rect_f title_rect = {SCREEN_CENTER, SCREEN_TOP - 20, 40, 20};
  this->title = create_sprite_g(FIND_ASSET("texture/main_menu/title.bmp"), title_rect);

  struct rect_f start_game_rect = {SCREEN_CENTER, SCREEN_CENTER, 8, 2};
  this->btn_start_game.icon = create_sprite_g(FIND_ASSET("texture/main_menu/start_game.bmp"), start_game_rect);
  this->btn_start_game.on_hovered = &buttom_on_hovered;
  this->btn_start_game.on_unhovered = &buttom_on_unhovered;
  this->btn_start_game.on_pressed = &buttom_on_start_game;

  struct rect_f fullscreen_rect = {SCREEN_CENTER, 40, 13, 3};
  this->btn_fullscreen = create_buttom_f(fullscreen_rect);
  this->btn_fullscreen.icon = create_sprite_g(FIND_ASSET("texture/main_menu/fullscreen.bmp"), fullscreen_rect);
  this->btn_fullscreen.on_hovered = &buttom_on_hovered;
  this->btn_fullscreen.on_unhovered = &buttom_on_unhovered;
  this->btn_fullscreen.on_pressed = &buttom_on_set_fullscreen;

  struct rect_f quit_game_rect = {SCREEN_CENTER, SCREEN_CENTER - 20, 13, 2};
  this->btn_quit_game = create_buttom_f(quit_game_rect);
  this->btn_quit_game.icon = create_sprite_g(FIND_ASSET("texture/main_menu/quit_game.bmp"), quit_game_rect);
  this->btn_quit_game.on_hovered = &buttom_on_hovered;
  this->btn_quit_game.on_unhovered = &buttom_on_unhovered;
  this->btn_quit_game.on_pressed = &buttom_on_quit_game;
}

void level_update(float delta_time) {
  if (is_key_pressed_f(KEY_ESCAPE)) {
    quit_game_p();
  }
  if (is_key_pressed_f(KEY_TAB)) {
    set_window_fullscreen_p();
  }

  update_buttom_f(&this->btn_start_game);
  update_buttom_f(&this->btn_fullscreen);
  update_buttom_f(&this->btn_quit_game);
}

void level_draw(float delta_time) {
  draw_sprite_g(this->background);
  draw_sprite_g(this->title);
  draw_sprite_g(this->credit);
  draw_sprite_g(this->btn_start_game.icon);
  draw_sprite_g(this->btn_fullscreen.icon);
  draw_sprite_g(this->btn_quit_game.icon);
}

void level_end() {
  destroy_sprite_g(this->title);
  destroy_sprite_g(this->credit);
  destroy_sprite_g(this->background);
  destroy_sprite_g(this->btn_start_game.icon);
  destroy_sprite_g(this->btn_fullscreen.icon);
  destroy_sprite_g(this->btn_quit_game.icon);
  free_memory_f(this);
}

void buttom_on_hovered(struct buttom_f *buttom) {
  buttom->icon.scale = vector3_add_scale(buttom->icon.scale, 1);
}

void buttom_on_unhovered(struct buttom_f *buttom) {
  buttom->icon.scale = vector3_sub_scale(buttom->icon.scale, 1);
}

void buttom_on_start_game(struct buttom_f *buttom) {
  open_level_f(load_level_1());
}

void buttom_on_set_fullscreen(struct buttom_f *buttom) {
  set_window_fullscreen_p();
}

void buttom_on_quit_game(struct buttom_f *buttom) {
  quit_game_p();
}
