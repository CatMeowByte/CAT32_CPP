#include "core/interpreter.hpp"
#include "core/video.hpp"
#include "lib/sdl.hpp"
#include "spec/base.hpp"

void fps();

vector<interpreter::Line> code;

void init() {
 ifstream file("/media/storage/share/cpp/CAT32/example/0.app");
 if (!file) {
  cerr << "Failed to open file.\n";
 }

 string line;
 // per line
 while (getline(file, line)) {
  code.push_back(interpreter::tokenize(line));
 }

 // print type with value
 for (u16 i = 0; i < code.size(); i++) {
  for (u16 j = 0; j < code[i].size(); j++) {
   if (j > 0) {cout << ' ';}
   cout << "[";
   if (code[i][j].type == interpreter::CMD) {cout << "CMD";}
   else if (code[i][j].type == interpreter::INT) {cout << "INT";}
   else if (code[i][j].type == interpreter::STR) {cout << "STR";}
   else {cout << "NIL";}
   cout << ":\"" << code[i][j].value << "\"]";
  }
  cout << endl;
 }
}

void update() {
 // per-frame logic
}

void draw() {
 for (const interpreter::Line& line : code) {
  interpreter::execute(line);
 }
 fps();

 video::flip();
}

int main() {
 if (!sdl::init()) {return 1;}
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