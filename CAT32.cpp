#include "core/constant.hpp"
#include "core/interpreter.hpp"
#include "core/kernel.hpp"
#include "core/memory.hpp"
#include "module/button.hpp"
#include "module/filesystem.hpp"
#include "module/video.hpp"
#include "library/sdl.hpp"


namespace kernel {
 void run_event(Event handler) {
  using namespace memory::vm;
  using namespace ram_global::constant;
  using namespace process::app::ram_local;
  counter = fpu(cast(u8, handler));
  while (counter < writer && sleeper == zero) {interpreter::step();}
 }
}

int main() {
 if (!sdl::init()) {return 1;}
 sdl::delay(1); // wait until ready

 // init
 memory::reset();

 cout << "\nLOAD ===============================\n" << endl;
 filesystem::load("/app/file_manager.app");
 cout << "\nRUN ===============================\n" << endl;
 filesystem::run();

 // internal ticker
 constexpr float update_interval = 1000.0f / TICK::UPDATE;
 constexpr float frame_interval = 1000.0f / TICK::DRAW;

 u32 tick_prev = sdl::get_ticks();
 float update_time = 0.0f;
 float draw_time = 0.0f;

 while (sdl::poll()) {
  u32 tick_now = sdl::get_ticks();
  float tick_delta = tick_now - tick_prev;
  tick_prev = tick_now;

  update_time += tick_delta;
  draw_time += tick_delta;

  while (update_time >= update_interval) {
   using namespace memory::vm::process::app::ram_local;
   if (sleeper) {sleeper--;}
   button::update();
   kernel::run_event(kernel::Event::Step);
   update_time -= update_interval;
  }

  if (draw_time >= frame_interval) {
   kernel::run_event(kernel::Event::Draw);
   video::flip(); // FIXME: need to queue
   draw_time = 0.0f;
  }

  sdl::delay(1);
 }

 sdl::shutdown();
 return 0;
}

void fps() {
 static float fps = 0;
 static u32 lastFrame = 0;

 u32 now = sdl::get_ticks();
 u32 delta = now - lastFrame;
 lastFrame = now;

 if (delta > 0) {
  fps = 1000.0f / delta;
 }

 char buf[5];
 snprintf(buf, sizeof(buf), "%.1f", fps);
 video::text((VIDEO::WIDTH - (4 * 4)) / 2, VIDEO::HEIGHT - 8, buf, 7, 0);
}