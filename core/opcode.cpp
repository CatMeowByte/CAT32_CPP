#include "core/module.hpp"
#include "core/memory.hpp"
#include "core/opcode.hpp"

namespace opcode {
 static const hash_map<string, u8> opcode_table = {
  #define OP(hex, name) {#name, op::name},
  OPCODES
  #undef OP
 };

 u8 get(string cmd) {
  return opcode_table.count(cmd) ? opcode_table.at(cmd) : op::nop;
 }

 bool exist(string cmd) {
  return opcode_table.count(cmd);
 }

 string name(u8 value) {
  switch (value) {
   #define OP(hex, name) case hex: return #name;
   OPCODES
   #undef OP
  }
  return "UNKNOWN";
 }
}

namespace opfunc {
 /* stack */
 addr push(fpu value) {
  BAIL_IF_STACK_OVERFLOW
  using namespace memory::vm::process::app::ram_local;
  field[--stacker] = value;
  return SENTINEL;
 }

 addr pop(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  using namespace memory::vm::process::app::ram_local;
  return field[stacker++];
 }

 /* memory */
 addr takefrom(fpu value) {
  BAIL_IF_STACK_OVERFLOW
  using namespace memory::vm::process::app;
  ram_local::field[--ram_local::stacker] = ram_local_fpu[value];
  return SENTINEL;
 }

 addr storeto(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  using namespace memory::vm::process::app;
  ram_local_fpu[value] = ram_local::field[ram_local::stacker++];
  return SENTINEL;
 }

 addr peek(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  using namespace memory::vm::process::app;
  addr address = memory::pop();
  push(fpu(cast(double, ram_local_octo[address])));
  return SENTINEL;
 }

 addr poke(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  using namespace memory::vm::process::app;
  fpu a = memory::pop();
  addr address = memory::pop();
  ram_local_octo[address] = cast(octo, a);
  return SENTINEL;
 }

 /* counter */
 addr subgo(fpu value) {
  using namespace memory::vm::process::app::ram_local;
  if (framer >= fpu(frames_length, true)) {return SENTINEL;}
  frames[framer++] = counter;
  return value;
 }

 addr subret(fpu value) {
  using namespace memory::vm::process::app::ram_local;
  if (framer == fpu(0, true)) {return SENTINEL;}
  addr address = frames[--framer];
  return address + 2;
 }

 addr jump(fpu value) {
  return value;
 }

 addr jumz(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  fpu check = memory::pop();
  if (!check) {return value;}
  return SENTINEL;
 }

 addr junz(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  fpu check = memory::pop();
  if (check) {return value;}
  return SENTINEL;
 }

 /* math */
 addr add(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a + b);
  return SENTINEL;
 }

 addr sub(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a - b);
  return SENTINEL;
 }

 addr mul(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a * b);
  return SENTINEL;
 }

 addr div(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(b ? a / b : fpu(0, true));
  return SENTINEL;
 }

 addr neg(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  fpu a = memory::pop();
  push(-a);
  return SENTINEL;
 }

 /* logic */
 addr eq(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(a == b));
  return SENTINEL;
 }

 addr neq(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(a != b));
  return SENTINEL;
 }

 addr gt(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(a > b));
  return SENTINEL;
 }

 addr lt(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(a < b));
  return SENTINEL;
 }

 addr geq(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(a >= b));
  return SENTINEL;
 }

 addr leq(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(a <= b));
  return SENTINEL;
 }

 addr land(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(cast(bool, a) && cast(bool, b)));
  return SENTINEL;
 }

 addr lor(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(cast(bool, a) || cast(bool, b)));
  return SENTINEL;
 }

 addr lnot(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  fpu a = memory::pop();
  push(fpu(!cast(bool, a)));
  return SENTINEL;
 }

 /* bit */
 addr band(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a & b);
  return SENTINEL;
 }

 addr bor(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a | b);
  return SENTINEL;
 }

 addr bnot(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  fpu a = memory::pop();
  push(~a);
  return SENTINEL;
 }

 addr bshl(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a << cast(s32, b));
  return SENTINEL;
 }

 addr bshr(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a >> cast(s32, b));
  return SENTINEL;
 }

 /* module */
 addr call(fpu value) {
  return module::table[cast(s32, value)].function(value);
 }

 /* nop */
 addr nop(fpu value) {
  return SENTINEL;
 }
}