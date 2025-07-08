#include "core/define.hpp"
#include "core/memory.hpp"
#include "module/opcode.hpp"
#include "module/video.hpp"

namespace opfunc {
 /* stack */
 addr push(elem value) {
  if (stacker > 0) {memory[--stacker] = value;}
  return SENTINEL;
 }

 addr pop(elem value) {
  if (stacker < SYSTEM::MEMORY) {return memory[stacker++];}
  return SENTINEL;
 }

 addr pushm(elem value) {
  if (stacker > 0) {memory[--stacker] = memory[value];}
  return SENTINEL;
 }

 addr popm(elem value) {
  if (stacker < SYSTEM::MEMORY) {
   memory[value] = memory[stacker++];
  }
  return SENTINEL;
 }

 /* counter */
 addr jump(elem value) {
  return value;
 }

 addr jumz(elem value) {
  elem check = pop(0);
  if (check == 0) {return value;}
  return SENTINEL;
 }

 addr junz(elem value) {
  elem check = pop(0);
  if (check != 0) {return value;}
  return SENTINEL;
 }

 /* math */
 addr add(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   elem b = pop(0);
   elem a = pop(0);
   push(a + b);
  }
  return SENTINEL;
 }

 addr sub(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   elem b = pop(0);
   elem a = pop(0);
   push(a - b);
  }
  return SENTINEL;
 }

 addr mul(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   s64 b = cast(s32, pop(0));
   s64 a = cast(s32, pop(0));
   push(cast(elem, fpu::unpack_wide(a * b)));
  }
  return SENTINEL;
 }

 addr div(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   s64 b = cast(s32, pop(0));
   s64 a = cast(s32, pop(0));
   if (b == 0) { // TODO: division zero
    push(0);
   }
   push(cast(elem, fpu::pack_wide(a) / b));
  }
  return SENTINEL;
 }

 /* logic */
 addr eq(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(a == b));
  }
  return SENTINEL;
 }

 addr neq(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(a != b));
  }
  return SENTINEL;
 }

 addr gt(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(a > b));
  }
  return SENTINEL;
 }

 addr lt(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(a < b));
  }
  return SENTINEL;
 }

 addr geq(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(a >= b));
  }
  return SENTINEL;
 }

 addr leq(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(a <= b));
  }
  return SENTINEL;
 }

 addr land(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(cast(bool, a) && cast(bool, b)));
  }
  return SENTINEL;
 }

 addr lor(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(cast(bool, a) || cast(bool, b)));
  }
  return SENTINEL;
 }

 addr lnot(elem value) {
  if (stacker < SYSTEM::MEMORY) {
   elem a = pop(0);
   push(fpu::pack(!cast(bool, a)));
  }
  return SENTINEL;
 }

 /* bit */
 addr band(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   elem b = pop(0);
   elem a = pop(0);
   push(a & b);
  }
  return SENTINEL;
 }

 addr bor(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   elem b = pop(0);
   elem a = pop(0);
   push(a | b);
  }
  return SENTINEL;
 }

 addr bnot(elem value) {
  if (stacker < SYSTEM::MEMORY) {
   elem a = pop(0);
   push(~a);
  }
  return SENTINEL;
 }

 addr bshl(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   elem b = pop(0);
   elem a = pop(0);
   push(a << fpu::unpack(b));
  }
  return SENTINEL;
 }

 addr bshr(elem value) {
  if (stacker < SYSTEM::MEMORY - 1) {
   elem b = pop(0);
   elem a = pop(0);
   push(a >> fpu::unpack(b));
  }
  return SENTINEL;
 }

 /* video */
 addr clear(elem value) {
  elem color = fpu::unpack(pop(0));
  video::clear(color);
  return SENTINEL;
 }

 addr pixel(elem value) {
  elem color = fpu::unpack(pop(0));
  elem y = fpu::unpack(pop(0));
  elem x = fpu::unpack(pop(0));
  video::pixel(x, y, color);
  return SENTINEL;
 }

 addr flip(elem value) {
  video::flip();
  return SENTINEL;
 }

 addr nop(elem value) {
  return SENTINEL;
 }
}