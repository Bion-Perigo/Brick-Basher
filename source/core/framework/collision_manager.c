#include "core.h"

bool check_collision_sprite_f(struct sprite_f a, struct sprite_f b) {
  float a_bottom = a.position.y - a.scale.y;
  float a_top = a.position.y + a.scale.y;
  float a_left = a.position.x - a.scale.x;
  float a_right = a.position.x + a.scale.x;

  float b_bottom = b.position.y - b.scale.y;
  float b_top = b.position.y + b.scale.y;
  float b_left = b.position.x - b.scale.x;
  float b_right = b.position.x + b.scale.x;

  bool coll_x = (b_left < a_right && a_left < b_right);
  bool coll_y = (b_top > a_bottom && a_top > b_bottom);

  if (coll_x && coll_y) {
    return true;
  }
  return false;
}

bool check_collision_sprite_pointer_f(struct sprite_f a, struct vector2_f v) {
  float a_bottom = a.position.y - a.scale.y;
  float a_top = a.position.y + a.scale.y;
  float a_left = a.position.x - a.scale.x;
  float a_right = a.position.x + a.scale.x;

  bool coll_x = (v.x < a_right && v.x > a_left);
  bool coll_y = (v.y > a_bottom && v.y < a_top);

  if (coll_x && coll_y) {
    return true;
  }

  return false;
}
