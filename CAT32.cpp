// #include "core/interpreter.hpp"
#include "core/video.hpp"
#include "lib/sdl.hpp"
#include "spec/base.hpp"

void fps();

const u8 STACK_SIZE = 32;

u8 memory[1024 * 8]; // 8 KiB

u8 stack[STACK_SIZE];
u8 stacker = 0;

enum Opcode : u8 {
 NOP = 0x00,
 PUSH = 0x11,
 POP = 0x12,
 CLEAR = 0xA0,
 PIXEL = 0xA1,
 FLIP = 0xAF,
};

vector<u8> bytecode = {
 PUSH, 0,
 CLEAR, NOP,
 PUSH, 1,
 PUSH, 4,
 PUSH, 7,
 PIXEL, NOP,
 FLIP, NOP,
};

namespace op {
 void push(u8 value) {
  if (stacker >= STACK_SIZE) {return;}
  stack[stacker++] = value;
 }
 u8 pop() {
  if (stacker <= 0) {return 0;}
  return stack[--stacker];
 }

 void clear() {
  u8 color = pop();
  video::clear(color);
 }

 void pixel() {
  u8 color = pop();
  u8 y = pop();
  u8 x = pop();
  video::pixel(x, y, color);
 }

 void flip() {
  video::flip();
 }
}

void init() {
 for (u32 counter = 0; counter < bytecode.size(); counter += 2) {
  u8 opcode = bytecode[counter];
  u8 operand = bytecode[counter + 1];

  switch (opcode) {
   case PUSH:
    op::push(operand);
    break;
   case CLEAR:
    op::clear();
    break;
   case PIXEL:
    op::pixel();
    break;
   case FLIP:
    op::flip();
    break;
   case NOP:
    break;
  }
 }
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