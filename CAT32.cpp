#include "core/constant.hpp"
#include "core/interpreter.hpp"
#include "core/kernel.hpp"
#include "core/memory.hpp"
#include "module/button.hpp"
#include "module/filesystem.hpp"
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

static void boot() {
 memory::reset();
 cout << "\nLOAD ===============================\n" << endl;
 filesystem::load("/app/file_manager.app");
 cout << "\nRUN ===============================\n" << endl;
 filesystem::run();
}

static void tick() {
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
   run_event(kernel::Event::Step);
   update_time -= update_interval;
  }

  if (draw_time >= frame_interval) {
   run_event(kernel::Event::Draw);
   sdl::flip(memory::vm::ram_global::framebuffer);
   draw_time = 0.0f;
  }

  sdl::delay(1);
 }
}

int main() {
 using namespace sdl;
 if (!init()) {return 1;}
 delay(1);

 boot();
 tick();

 shutdown();
 return 0;
}