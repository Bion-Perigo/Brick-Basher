#include "core.h"

// Game Call
extern void game_start();
extern void game_update(float delta_time);
extern void game_draw(float delta_time);
extern void game_stop();

int brick_main(int argc, char **argv) {
  const float aspect = 1.7777;
  const int width = 1366;
  const int height = width / aspect;
  int target_fps = 60;

  init_window_p(width, height, "Brick-Basher");
  set_target_fps_f(target_fps);
  set_show_cursor_p(false);

  game_start();

  while (!window_should_close_p()) {
    begin_frame_p();
    clear_background_g(DARK_GRAY);
    game_update(get_frametime_f());
    game_draw(get_frametime_f());
    end_frame_p();
  }

  game_stop();

  close_window_p();
  return 0;
}

#if defined(PLATFORM_WINDOWS) && defined(RELEASE_MODE)
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  return brick_main(0, (char **)lpCmdLine);
}
#elif defined(PLATFORM_LINUX) || defined(DEBUG_MODE)
int main(int argc, char *argv[]) {
  brick_main(argc, argv);
}
#endif // PLATFORM_LINUX OR DEBUG_MODE
