#include "module/interpreter.hpp"
#include "module/opcode.hpp"
#include "module/video.hpp"
#include "library/sdl.hpp"
#include "core/constant.hpp"
#include "core/memory.hpp"

void fps();

void init() {
 memset(bytecode, op::nop, sizeof(bytecode));
 writer = 0;

 ifstream file("/media/storage/share/cpp/CAT32/example/5.app");
 if (!file) {
  cerr << "Failed to open file." << endl;
 }

 string line;
 // per line
 while (getline(file, line)) {
  interpreter::compile(interpreter::tokenize(line));
  cout << endl;
 }

 counter = 0;
 while (counter < writer) {interpreter::step();}
}

void update() {
}

void draw() {
}

int main() {
 if (!sdl::init()) {return 1;}
 sdl::delay(1); // wait until ready

 init();

 constexpr float update_interval = 1000.0f / TICK::UPDATE; // ~66.666 ms
 constexpr float frame_interval = 1000.0f / TICK::DRAW; // ~8.333 ms

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
   update();
   update_time -= update_interval;
  }

  if (draw_time >= frame_interval) {
   draw();
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