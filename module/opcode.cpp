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

 /* memory */
 addr takefrom(elem value) {
  if (stacker > 0) {memory[--stacker] = memory[value];}
  return SENTINEL;
 }

 addr storeto(elem value) {
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

 /* misc */

 addr see(elem value) {
  elem literal_value = pop(0);
  float decimal_value = fpu::unscale(literal_value);

  // format hex
  ostringstream hex_out;
  hex_out.setf(ios::uppercase);
  hex_out << hex;
  hex_out << setw(8);
  hex_out << setfill('0');
  hex_out << literal_value;
  string hex_string = hex_out.str();

  int dot_position = SYSTEM::FIXED_POINT_WIDTH / 4;
  string fixed_hex = hex_string.substr(0, 8 - dot_position) + "." + hex_string.substr(8 - dot_position);

  // format float
  ostringstream float_out;
  float_out << decimal_value;
  string decimal_string = float_out.str();

  cout << "[SEE] [" << literal_value << "] [" << fixed_hex << "] [" << decimal_string << "]" << endl;
  return SENTINEL;
 }

 addr wait(elem value) {
  sleeper = fpu::unpack(pop(0));
  return SENTINEL;
 }

 addr nop(elem value) {
  return SENTINEL;
 }
}