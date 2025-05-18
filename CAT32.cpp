#include "CAT32.hpp"

#include "lib/sdl.hpp"
#include "spec/spec.hpp"

int main() {
 if (!sdl_init()) {return 1;}
 init();

 u32 tick_prev = sdl_get_ticks();
 float update_time = 0.0f;
 float draw_time = 0.0f;

 // Calculate intervals inside main
 constexpr float update_interval = 1000.0f / TICK_UPDATE; // ~66.666 ms
 constexpr float frame_interval = 1000.0f / TICK_DRAW; // ~8.333 ms

 while (sdl_poll()) {
  u32 tick_now = sdl_get_ticks();
  float tick_delta = tick_now - tick_prev;
  tick_prev = tick_now;

  update_time += tick_delta;
  draw_time += tick_delta;

  while (update_time >= update_interval) {
   update();
   update_time -= update_interval;
  }

  if (draw_time >= frame_interval) {
   draw();
   draw_time = 0.0f;
  }

  sdl_delay(16); // 60 Hz
 }

 sdl_shutdown();
 return 0;
}