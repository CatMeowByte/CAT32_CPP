#include "core/constant.hpp"
#include "core/interpreter.hpp"
#include "core/memory.hpp"
#include "core/utility.hpp"
#include "module/button.hpp"
#include "module/video.hpp"
#include "library/sdl.hpp"

enum class Event : u8 {Load = 15, Init = 0, Step = 5, Draw = 10};

void event_run(Event handler) {
 using namespace memory::vm;
 using namespace ram_global::constant;
 using namespace process::app::ram_local;
 counter = fpu(cast(u8, handler));
 while (counter < writer && sleeper == zero) {interpreter::step();}
}

int main() {
 if (!sdl::init()) {return 1;}
 sdl::delay(1); // wait until ready

 // init
 memory::reset();

 button::module_register();
 video::module_register();
 utility::module_register();

 ifstream file("/media/storage/share/cpp/CAT32/example/packedstring.app");
 if (!file) {cerr << "Failed to open file." << endl;}

 string line;
 interpreter::TokenLine tokenline = {};
 // per line
 while (getline(file, line)) {
  tokenline = interpreter::tokenize(line);
  interpreter::compile(tokenline);

  if (tokenline.tokens.empty()) {cout << line << endl;}
  else {cout << endl;}
 }

 // dedent
 tokenline = interpreter::tokenize("wait(0)"); // TODO: probably need a better take on this
 interpreter::compile(tokenline);

 cout << "\nBEGIN ===============================\n" << endl;

 event_run(Event::Load);
 event_run(Event::Init);

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
   event_run(Event::Step);
   update_time -= update_interval;
  }

  if (draw_time >= frame_interval) {
   event_run(Event::Draw);
   video::flip();
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