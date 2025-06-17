#include "core/memory.hpp"
#include "module/opcode.hpp"
#include "module/video.hpp"

namespace op {
 u8 push(u8 value) {
  if (stacker > 0) {memory[--stacker] = value;}
  return 0;
 }

 u8 pop(u8 value) {
  if (stacker < SYSTEM::MEMORY) {return memory[stacker++];}
  return 0;
 }

 u8 pushm(u8 value) {
  if (stacker > 0) {memory[--stacker] = memory[value];}
  return 0;
 }

 u8 popm(u8 value) {
  if (stacker < SYSTEM::MEMORY) {
   memory[value] = memory[stacker++];
  }
  return 0;
 }

 u8 clear(u8 value) {
  u8 color = pop(0);
  video::clear(color);
  return 0;
 }

 u8 pixel(u8 value) {
  u8 color = pop(0);
  u8 y = pop(0);
  u8 x = pop(0);
  video::pixel(x, y, color);
  return 0;
 }

 u8 flip(u8 value) {
  video::flip();
  return 0;
 }

 u8 nop(u8 value) {
  return 0;
 }
}