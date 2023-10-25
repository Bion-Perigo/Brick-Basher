#include "core.h"

#include <math.h>

struct mat4_f matrix_identity_f() {
  struct mat4_f matrix;

  matrix.e[0][0] = 1.f;
  matrix.e[0][1] = 0.f;
  matrix.e[0][2] = 0.f;
  matrix.e[0][3] = 0.f; //
  matrix.e[1][0] = 0.f;
  matrix.e[1][1] = 1.f;
  matrix.e[1][2] = 0.f;
  matrix.e[1][3] = 0.f; //
  matrix.e[2][0] = 0.f;
  matrix.e[2][1] = 0.f;
  matrix.e[2][2] = 1.f;
  matrix.e[2][3] = 0.f; //
  matrix.e[3][0] = 0.f;
  matrix.e[3][1] = 0.f;
  matrix.e[3][2] = 0.f;
  matrix.e[3][3] = 1.f; //

  return matrix;
}

struct mat4_f matrix_mult_f(struct mat4_f a, struct mat4_f b) {
  struct mat4_f m = {0};

  for (int line = 0; line < 4; line++) {

    for (int column = 0; column < 4; column++) {
      m.e[column][line] = a.e[0][line] * b.e[column][0] + a.e[1][line] * b.e[column][1] +
                          a.e[2][line] * b.e[column][2] + a.e[3][line] * b.e[column][3];
    }
  }

  return m;
}

struct mat4_f matrix_init_perspective_f(float fov, float aspect, float z_near, float z_far) {
  struct mat4_f mat = matrix_identity_f();
  const float tan_half_fov = (float)tan(fov / 2);

  mat.e[0][0] = 1 / (aspect * tan_half_fov);
  mat.e[1][1] = 1 / tan_half_fov;
  mat.e[2][2] = -(z_far + z_near) / (z_far - z_near);
  mat.e[2][3] = -1;
  mat.e[3][2] = -(2 * z_far * z_near) / (z_far - z_near);

  return mat;
}

struct mat4_f matrix_init_ortho_f(float left, float right, float top, float bottom, float z_near, float z_far) {
  struct mat4_f mat = matrix_identity_f();

  mat.e[0][0] = 2 / (right - left);
  mat.e[1][1] = 2 / (top - bottom);
  mat.e[3][3] = -2 / (z_far - z_near);
  mat.e[3][0] = -(right + left) / (right - left);
  mat.e[3][1] = -(top + bottom) / (top - bottom);
  mat.e[3][2] = -(z_far + z_near) / (z_far - z_near);
  mat.e[3][3] = 1;

  return mat;
}

struct mat4_f matrix_init_translation_f(float x, float y, float z) {
  struct mat4_f m = matrix_identity_f();

  m.e[3][0] = x;
  m.e[3][1] = y;
  m.e[3][2] = z;

  return m;
}

struct mat4_f matrix_init_rotation_f(float x, float y, float z) {
  const double toRad = (M_PI / 180.0);

  const double angleX = x * toRad;
  const double angleY = y * toRad;
  const double angleZ = z * toRad;

  const double cx = cos(angleX);
  const double cy = cos(angleY);
  const double cz = cos(angleZ);

  const double sx = sin(angleX);
  const double sy = sin(angleY);
  const double sz = sin(angleZ);

  // Right-hand rotation matrices
  struct mat4_f xRot = matrix_identity_f();
  xRot.e[1][1] = (float)(cx);
  xRot.e[1][2] = (float)(sx);
  xRot.e[2][1] = (float)(-sx);
  xRot.e[2][2] = (float)(cx);

  struct mat4_f yRot = matrix_identity_f();
  yRot.e[0][0] = (float)(cy);
  yRot.e[0][2] = (float)(-sy);
  yRot.e[2][0] = (float)(sy);
  yRot.e[2][2] = (float)(cy);

  struct mat4_f zRot = matrix_identity_f();
  zRot.e[0][0] = (float)(cz);
  zRot.e[0][1] = (float)(sz);
  zRot.e[1][0] = (float)(-sz);
  zRot.e[1][1] = (float)(cz);

  return matrix_mult_f(zRot, matrix_mult_f(yRot, xRot));
}

struct mat4_f matrix_init_scale_f(float x, float y, float z) {
  struct mat4_f m = {0};

  m.e[0][0] = x;
  m.e[1][1] = y;
  m.e[2][2] = z;
  m.e[3][3] = 1.f;

  return m;
}
