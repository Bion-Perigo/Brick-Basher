#include "core.h"

#include <time.h>

static int fps_current = 0;
static float fps_ms = 0.f;
static float fps_target = 0;
static float begin_time = 0;
static float end_time = 0;

void begin_time_f() {
  begin_time = get_time_p();
}

void end_time_f() {
  end_time = get_time_p();

  fps_ms = end_time - begin_time;
  if(fps_target != 0){
    while(fps_ms <= fps_target){
      // Matando o tempo pra ele não me matar \_(-_-)_/
      fps_ms = get_time_p() - begin_time;
    }
  }
  fps_current = 1 / fps_ms;
}

void set_target_fps_f(int max_fps) {
  fps_target = (max_fps != 0) ? 1.f / max_fps : 0;
  G_LOG(LOG_INFO, "TIMER: Target FPS:%d, Ms:%f", max_fps, fps_target);
}

int get_fps_f() {
  return fps_current;
}

float get_frametime_f() {
  return fps_ms;
}
