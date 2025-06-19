#include "core/memory.hpp"
#include "module/opcode.hpp"
#include "module/video.hpp"

namespace opfunc {
 u32 push(u32 value) {
  if (stacker > 0) {memory[--stacker] = value;}
  return 0;
 }

 u32 pop(u32 value) {
  if (stacker < SYSTEM::MEMORY) {return memory[stacker++];}
  return 0;
 }

 u32 pushm(u32 value) {
  if (stacker > 0) {memory[--stacker] = memory[value];}
  return 0;
 }

 u32 popm(u32 value) {
  if (stacker < SYSTEM::MEMORY) {
   memory[value] = memory[stacker++];
  }
  return 0;
 }

 u32 add(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(a + b);
  }
  return 0;
 }

 u32 sub(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(a - b);
  }
  return 0;
 }

 u32 mul(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(a * b);
  }
  return 0;
 }

 u32 div(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   if (b == 0) { // TODO: division zero
    push(0);
   }
   push(a / b);
  }
  return 0;
 }

 u32 clear(u32 value) {
  u32 color = pop(0);
  video::clear(color);
  return 0;
 }

 u32 pixel(u32 value) {
  u32 color = pop(0);
  u32 y = pop(0);
  u32 x = pop(0);
  video::pixel(x, y, color);
  return 0;
 }

 u32 flip(u32 value) {
  video::flip();
  return 0;
 }

 u32 nop(u32 value) {
  return 0;
 }
}