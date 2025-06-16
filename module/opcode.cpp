#include "core/memory.hpp"
#include "module/opcode.hpp"
#include "module/video.hpp"

namespace op {
 void push(u8 value) {
  if (stacker > 0) {memory[--stacker] = value;}
 }
 u8 pop() {
  if (stacker < SYSTEM::MEMORY) {return memory[stacker++];}
  return 0;
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