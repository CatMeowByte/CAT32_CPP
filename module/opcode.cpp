#include "core/memory.hpp"
#include "module/opcode.hpp"
#include "module/video.hpp"

namespace opfunc {
 /* stack */
 u32 push(u32 value) {
  if (stacker > 0) {memory[--stacker] = value;}
  return SENTINEL;
 }

 u32 pop(u32 value) {
  if (stacker < SYSTEM::MEMORY) {return memory[stacker++];}
  return SENTINEL;
 }

 u32 pushm(u32 value) {
  if (stacker > 0) {memory[--stacker] = memory[value];}
  return SENTINEL;
 }

 u32 popm(u32 value) {
  if (stacker < SYSTEM::MEMORY) {
   memory[value] = memory[stacker++];
  }
  return SENTINEL;
 }

 /* counter */
 u32 jump(u32 value) {
  return value;
 }

 u32 jumz(u32 value) {
  u32 check = pop(0);
  if (check == 0) {return value;}
  return SENTINEL;
 }

 u32 junz(u32 value) {
  u32 check = pop(0);
  if (check != 0) {return value;}
  return SENTINEL;
 }

 /* math */
 u32 add(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(a + b);
  }
  return SENTINEL;
 }

 u32 sub(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(a - b);
  }
  return SENTINEL;
 }

 u32 mul(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(a * b);
  }
  return SENTINEL;
 }

 u32 div(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   s32 b = pop(0);
   s32 a = pop(0);
   if (b == 0) { // TODO: division zero
    push(0);
   }
   push(a / b);
  }
  return SENTINEL;
 }

 /* logic */
 u32 eq(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(a == b);
  }
  return SENTINEL;
 }

 u32 neq(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(a != b);
  }
  return SENTINEL;
 }

 u32 gt(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(a > b);
  }
  return SENTINEL;
 }

 u32 lt(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(a < b);
  }
  return SENTINEL;
 }

 u32 geq(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(a >= b);
  }
  return SENTINEL;
 }

 u32 leq(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(a <= b);
  }
  return SENTINEL;
 }

 u32 land(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(cast(bool, a) && cast(bool, b));
  }
  return SENTINEL;
 }

 u32 lor(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(cast(bool, a) || cast(bool, b));
  }
  return SENTINEL;
 }

 u32 lnot(u32 value) {
  if (stacker < SYSTEM::MEMORY) {
   u32 a = pop(0);
   push(!cast(bool, a));
  }
  return SENTINEL;
 }

 /* bit */
 u32 band(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(a & b);
  }
  return SENTINEL;
 }

 u32 bor(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   u32 b = pop(0);
   u32 a = pop(0);
   push(a | b);
  }
  return SENTINEL;
 }

 u32 bnot(u32 value) {
  if (stacker < SYSTEM::MEMORY) {
   u32 a = pop(0);
   push(~a);
  }
  return SENTINEL;
 }

 u32 bshl(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   s32 b = pop(0);
   s32 a = pop(0);
   push(a << b);
  }
  return SENTINEL;
 }

 u32 bshr(u32 value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   s32 b = pop(0);
   s32 a = pop(0);
   push(a >> b);
  }
  return SENTINEL;
 }

 /* video */
 u32 clear(u32 value) {
  u32 color = pop(0);
  video::clear(color);
  return SENTINEL;
 }

 u32 pixel(u32 value) {
  u32 color = pop(0);
  u32 y = pop(0);
  u32 x = pop(0);
  video::pixel(x, y, color);
  return SENTINEL;
 }

 u32 flip(u32 value) {
  video::flip();
  return SENTINEL;
 }

 u32 nop(u32 value) {
  return SENTINEL;
 }
}