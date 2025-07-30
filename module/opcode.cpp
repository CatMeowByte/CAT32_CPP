#include "core/constant.hpp"
#include "core/define.hpp"
#include "core/memory.hpp"
#include "module/opcode.hpp"
#include "module/video.hpp"

#define STACK_NOT_OVERFLOW if (stacker > 0)
#define STACK_AT_LEAST(N) if (stacker <= SYSTEM::MEMORY - (N))

namespace opfunc {
 /* stack */
 addr push(elem value) {
  STACK_NOT_OVERFLOW {memory[--stacker] = value;}
  return SENTINEL;
 }

 addr pop(elem value) {
  STACK_AT_LEAST(1) {return memory[stacker++];}
  return SENTINEL;
 }

 /* memory */
 addr takefrom(elem value) {
  STACK_NOT_OVERFLOW {memory[--stacker] = memory[value];}
  return SENTINEL;
 }

 addr storeto(elem value) {
  STACK_AT_LEAST(1) {
   memory[value] = memory[stacker++];
  }
  return SENTINEL;
 }

 addr peek(elem value) {
  STACK_AT_LEAST(1) {
   addr address = fpu::unpack(pop(0));
   push(memory[address]);
  }
  return SENTINEL;
 }

 addr poke(elem value) {
  STACK_AT_LEAST(2) {
   elem a = pop(0);
   addr address = fpu::unpack(pop(0));
   memory[address] = a;
  }
  return SENTINEL;
 }

 /* counter */
 addr subgo(elem value) {
  framer.push_back(counter);
  return value;
 }

 addr subret(elem value) {
  if (framer.empty()) {return SENTINEL;}
  addr address = framer.back();
  framer.pop_back();
  return address + 2;
 }

 addr jump(elem value) {
  return value;
 }

 addr jumz(elem value) {
  STACK_AT_LEAST(1) {
   elem check = pop(0);
   if (check == 0) {return value;}
  }
  return SENTINEL;
 }

 addr junz(elem value) {
  STACK_AT_LEAST(1) {
   elem check = pop(0);
   if (check != 0) {return value;}
  }
  return SENTINEL;
 }

 /* math */
 addr add(elem value) {
  STACK_AT_LEAST(2) {
   elem b = pop(0);
   elem a = pop(0);
   push(a + b);
  }
  return SENTINEL;
 }

 addr sub(elem value) {
  STACK_AT_LEAST(2) {
   elem b = pop(0);
   elem a = pop(0);
   push(a - b);
  }
  return SENTINEL;
 }

 addr mul(elem value) {
  STACK_AT_LEAST(2) {
   s64 b = cast(s32, pop(0));
   s64 a = cast(s32, pop(0));
   push(cast(elem, fpu::unpack_wide(a * b)));
  }
  return SENTINEL;
 }

 addr div(elem value) {
  STACK_AT_LEAST(2) {
   s64 b = cast(s32, pop(0));
   s64 a = cast(s32, pop(0));
   if (b == 0) {
    push(SENTINEL);
   }
   push(cast(elem, fpu::pack_wide(a) / b));
  }
  return SENTINEL;
 }

 addr neg(elem value) {
  STACK_AT_LEAST(1) {
   elem a = pop(0);
   push(-a);
  }
  return SENTINEL;
 }

 /* logic */
 addr eq(elem value) {
  STACK_AT_LEAST(2) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(a == b));
  }
  return SENTINEL;
 }

 addr neq(elem value) {
  STACK_AT_LEAST(2) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(a != b));
  }
  return SENTINEL;
 }

 addr gt(elem value) {
  STACK_AT_LEAST(2) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(a > b));
  }
  return SENTINEL;
 }

 addr lt(elem value) {
  STACK_AT_LEAST(2) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(a < b));
  }
  return SENTINEL;
 }

 addr geq(elem value) {
  STACK_AT_LEAST(2) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(a >= b));
  }
  return SENTINEL;
 }

 addr leq(elem value) {
  STACK_AT_LEAST(2) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(a <= b));
  }
  return SENTINEL;
 }

 addr land(elem value) {
  STACK_AT_LEAST(2) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(cast(bool, a) && cast(bool, b)));
  }
  return SENTINEL;
 }

 addr lor(elem value) {
  STACK_AT_LEAST(2) {
   elem b = pop(0);
   elem a = pop(0);
   push(fpu::pack(cast(bool, a) || cast(bool, b)));
  }
  return SENTINEL;
 }

 addr lnot(elem value) {
  STACK_AT_LEAST(1) {
   elem a = pop(0);
   push(fpu::pack(!cast(bool, a)));
  }
  return SENTINEL;
 }

 /* bit */
 addr band(elem value) {
  STACK_AT_LEAST(2) {
   elem b = pop(0);
   elem a = pop(0);
   push(a & b);
  }
  return SENTINEL;
 }

 addr bor(elem value) {
  STACK_AT_LEAST(2) {
   elem b = pop(0);
   elem a = pop(0);
   push(a | b);
  }
  return SENTINEL;
 }

 addr bnot(elem value) {
  STACK_AT_LEAST(1) {
   elem a = pop(0);
   push(~a);
  }
  return SENTINEL;
 }

 addr bshl(elem value) {
  STACK_AT_LEAST(2) {
   elem b = pop(0);
   elem a = pop(0);
   push(a << fpu::unpack(b));
  }
  return SENTINEL;
 }

 addr bshr(elem value) {
  STACK_AT_LEAST(2) {
   elem b = pop(0);
   elem a = pop(0);
   push(a >> fpu::unpack(b));
  }
  return SENTINEL;
 }

 /* video */
 addr clear(elem value) {
  STACK_AT_LEAST(1) {
   elem color = fpu::unpack(pop(0));
   video::clear(color);
  }
  return SENTINEL;
 }

 addr pixel(elem value) {
  STACK_AT_LEAST(3) {
   elem color = fpu::unpack(pop(0));
   elem y = fpu::unpack(pop(0));
   elem x = fpu::unpack(pop(0));
   video::pixel(x, y, color);
  }
  return SENTINEL;
 }

 addr text(elem value) {
  STACK_AT_LEAST(3) {
   elem color = fpu::unpack(pop(0));
   elem address = fpu::unpack(pop(0));
   elem y = fpu::unpack(pop(0));
   elem x = fpu::unpack(pop(0));
   video::text(x, y, utility::string_pick(address).c_str(), color, 0); // FIXME: need fixed character
  }
  return SENTINEL;
 }

 addr flip(elem value) {
  video::flip();
  return SENTINEL;
 }

 /* misc */

 addr see(elem value) {
  STACK_AT_LEAST(1) {
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
   string decimal_string = utility::string_no_trailing(decimal_value);

   cout << "[SEE] [" << literal_value << "] [" << fixed_hex << "] [" << decimal_string << "]" << endl;
  }
   return SENTINEL;
 }

 addr wait(elem value) {
  STACK_AT_LEAST(1) {sleeper = fpu::unpack(pop(0));}
  return SENTINEL;
 }

 addr nop(elem value) {
  return SENTINEL;
 }
}