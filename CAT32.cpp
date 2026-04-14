#include "core/constant.hpp"
#include "core/kernel.hpp"
#include "core/memory.hpp"
#include "module/button.hpp"
#include "library/sdl.hpp"

static void boot() {
 memory::reset();
 kernel::run("/app/file_manager.app");
 kernel::run("/app/input_bar.app", 1);
}

static void tick() {
 constexpr double update_interval = 1000.0 / TICK::UPDATE;
 constexpr double frame_interval = 1000.0 / TICK::DRAW;

 u64 tick_prev = sdl::get_ticks();
 double update_time = 0;
 double draw_time = 0;

 while (sdl::poll()) {
  u64 tick_now = sdl::get_ticks();
  double tick_delta = tick_now - tick_prev;
  tick_prev = tick_now;

  update_time += tick_delta;
  draw_time += tick_delta;

  while (update_time >= update_interval) {
   button::update();
   for (s8 i = SYSTEM::PROCESS - 1; i >= 0; i--) {
    active::index(i);
    run_event(kernel::Event::Step);
   }
   update_time -= update_interval;
  }

  if (draw_time >= frame_interval) {
   for (s8 i = 0; i < SYSTEM::PROCESS; i++) {
    active::index(i);
    run_event(kernel::Event::Draw);
   }
   sdl::flip();
   draw_time = 0;
  }

  sdl::delay(1);
 }
}

int main() {
 using namespace memory::vm::global;
 if (!sdl::audio(hardware_io::frequency)) {return 1;}
 if (!sdl::video(framebuffer)) {return 1;}
 sdl::delay(1);

 boot();
 tick();

 sdl::stop();
 return 0;
}