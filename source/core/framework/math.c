#include "core.h"

float map_range_f(float in_a, float in_b, float out_a, float out_b, float value) {
  return out_a + (value - in_a) * (out_b - out_a) / (in_b - in_a);
}

// Vector3

struct vector3_f vector3_add_scale(struct vector3_f v, float scale) {
  v.x += scale;
  v.y += scale;
  v.z += scale;
  return v;
}
struct vector3_f vector3_sub_scale(struct vector3_f v, float scale) {
  v.x -= scale;
  v.y -= scale;
  v.z -= scale;
  return v;
}
