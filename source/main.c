#include "GL/glcorearb.h"
#include "core.h"

int main(int argc, char *argv[]) {
  const float aspect = 1.7777;
  const int width = 1024;
  const int height = width / aspect;
  int target_fps = 60;
  bool b_show_cursor = true;

  init_window_p(width, height, "Brick-Basher");
  set_target_fps_f(target_fps);

  while (!window_should_close_p()) {
    begin_frame_p();
    graphic_clear_f(SKY_BLUE);

    if (is_key_pressed_f(KEY_ENTER)) {
      b_show_cursor = !b_show_cursor;
      set_show_cursor_p(b_show_cursor);
      G_LOG(LOG_INFO, "Show Cursor:%s", (b_show_cursor) ? "True" : "false");
    }
    if (is_key_pressed_f(KEY_TAB)) {
      set_window_fullscreen_p();
    }
    if (is_key_pressed_f(KEY_ESCAPE)) {
      quit_game_p();
    }

    end_frame_p();
  }

  close_window_p();
}
