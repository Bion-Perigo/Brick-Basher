#include "level_1.h"

struct level_f load_level_1(){
    struct level_f level = {0};
    level.on_level_begin = &level_1_begin;
    level.on_level_update = &level_1_update;
    level.on_level_draw = &level_1_draw;
    level.on_level_end = &level_1_end;
		return level;
}

// Variables
#define BRICK_COUNT 27
struct sprite_f level;
struct sprite_f player;
struct sprite_f ball;
struct sprite_f bricks[BRICK_COUNT];
struct texture_f brick_texture = {0};
float normal_speed = 50.f;
float max_speed = 100.f;
float gravity = 0.1f;
struct vector2_f ball_speed = {100.f};

bool ball_stopped = true;
float right_value = 0;

void level_1_begin(){
// Init Variables


// Create Sprites
  struct rect_f level_rect = {SCREEN_LEFT + 50, SCREEN_BOTTON + 50, 50,50};
  level = create_sprite_g(FIND_ASSET("texture/actors/level_2_background.bmp"), level_rect);
  level.position.z = -1; // Set Backgroud Position
    
  struct rect_f player_rect = {50.f, 2.f , 6.f, 3.f};
  player = create_sprite_g(FIND_ASSET("texture/actors/player.bmp"), player_rect);

  struct rect_f ball_rect = {0.f, 0.f, 2.f, 2.f};
  ball = create_sprite_g(FIND_ASSET("texture/actors/ball.bmp"), ball_rect);

  struct rect_f brick_rect[BRICK_COUNT] = {0};
  brick_texture = load_texture_f(FIND_ASSET("texture/actors/brick.bmp"));

  struct vector2_f brick_size = {4,2};
  float min_distance = 10.f;
  int per_line = 9;
  int colluns = 0;
  int lines = 1;
  for(int i = 0; i < BRICK_COUNT; i++){
    float pos_x = (colluns == 0) ? min_distance:0 + SCREEN_LEFT + colluns * min_distance + brick_size.x; 
    float pos_y = SCREEN_TOP - lines * 6;
    colluns++;
    if(colluns == per_line) {
      colluns = 0;
      lines++;
    }
    brick_rect[i] = (struct rect_f){pos_x, pos_y, brick_size.x, brick_size.y};
    bricks[i] = (struct sprite_f)create_sprite_from_texture_g(brick_texture, brick_rect[i]);
  }

}

void level_1_update(float delta_time){

    if(is_key_pressed_f(KEY_ESCAPE)){
      quit_game_p();
    }
    if(is_key_pressed_f(KEY_TAB)){
      set_window_fullscreen_p();
    }

    if(is_key_repeat_f(KEY_D)){
      right_value = 1;
      player.position.x += 100.f * right_value * get_frametime_f();
    }
    if(is_key_repeat_f(KEY_A)){
      right_value = -1;
      player.position.x += 100.f * right_value * get_frametime_f();
    }
    if(is_key_pressed_f(KEY_SPACE) && ball_stopped){
      ball_stopped = false;
      ball_speed.x = max_speed * right_value;
      ball_speed.y = max_speed;
    }
    G_LOG(LOG_INFO, "Player Position X:%f, Y:%f, Z:%f", player.position.x, player.position.y, player.position.z);
    for(int i = 0; i < BRICK_COUNT; i++){
      if(check_collision_sprite_f(ball, bricks[i])){
        bricks[i].position.y = SCREEN_TOP + 100;
      }
    }

    if(check_collision_sprite_f(player, ball)){
      ball_speed.x = max_speed * right_value;
      ball_speed.y = max_speed;
    }

    if(player.position.x - player.scale.x < SCREEN_LEFT){
      player.position.x = SCREEN_LEFT + player.scale.x;
    }else if(player.position.x + player.scale.x > SCREEN_RIGHT){
      player.position.x = SCREEN_RIGHT - player.scale.x;
    }
   
    if(player.position.y - player.scale.y < SCREEN_BOTTON){
      player.position.y = SCREEN_BOTTON + player.scale.y;
    }else if(player.position.y + player.scale.y > SCREEN_TOP){
      player.position.y = SCREEN_TOP - player.scale.y;
    }

    if(ball.position.x < SCREEN_LEFT){
      ball_speed.x = normal_speed * 1;
    }else if(ball.position.x + ball.scale.x > SCREEN_RIGHT){
      ball_speed.x = normal_speed * -1;
    }
    if(ball.position.y < SCREEN_BOTTON){
      ball_stopped = true;
    }else if(ball.position.y + ball.scale.y > SCREEN_TOP){
      ball_speed.y = normal_speed * -1;
    }

    if(is_key_pressed_f(KEY_BACKSPACE)){
      destroy_texture_g(player.texture);
    }


    if(ball_stopped){
      ball.position.x = player.position.x;
      ball.position.y = player.position.y + player.scale.y + ball.scale.y;
    }else{
      ball.position.x += ball_speed.x * get_frametime_f();
      ball.position.y += ball_speed.y * get_frametime_f();
      ball.position.y -= gravity;
      ball.rotation.roll += 10.f * right_value;
    }

}

void level_1_draw(float delta_time){
    draw_sprite_g(level);
    draw_sprite_g(player);
    draw_sprite_g(ball);

    for(int i = 0; i < BRICK_COUNT; i++){
      draw_sprite_g(bricks[i]);
    }
}

void level_1_end(){
  destroy_sprite_g(level);
  destroy_sprite_g(player);
  destroy_sprite_g(ball);
  for(int i = 0; i < BRICK_COUNT; i++){
    destroy_sprite_g(bricks[i]);
  }
}