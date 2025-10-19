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
  OPDONE;
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
  OPDONE;
 }

 addr storeto(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  using namespace memory::vm::process::app;
  ram_local_fpu[value] = ram_local::field[ram_local::stacker++];
  OPDONE;
 }

 addr peek(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  using namespace memory::vm::process::app;
  addr address = memory::pop();
  push(fpu(cast(double, ram_local_octo[address])));
  OPDONE;
 }

 addr poke(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  using namespace memory::vm::process::app;
  fpu a = memory::pop();
  addr address = memory::pop();
  ram_local_octo[address] = cast(octo, a);
  OPDONE;
 }

 /* counter */
 addr subgo(fpu value) {
  using namespace memory::vm::process::app::ram_local;
  if (framer >= fpu(frames_length, true)) {OPDONE;}
  frames[framer++] = counter;
  return value;
 }

 addr subret(fpu value) {
  using namespace memory::vm::process::app::ram_local;
  if (framer == fpu(0, true)) {OPDONE;}
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
  OPDONE;
 }

 addr junz(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  fpu check = memory::pop();
  if (check) {return value;}
  OPDONE;
 }

 /* math */
 addr add(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a + b);
  OPDONE;
 }

 addr sub(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a - b);
  OPDONE;
 }

 addr mul(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a * b);
  OPDONE;
 }

 addr div(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(b ? a / b : fpu(0, true));
  OPDONE;
 }

 addr neg(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  fpu a = memory::pop();
  push(-a);
  OPDONE;
 }

 /* logic */
 addr eq(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(a == b));
  OPDONE;
 }

 addr neq(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(a != b));
  OPDONE;
 }

 addr gt(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(a > b));
  OPDONE;
 }

 addr lt(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(a < b));
  OPDONE;
 }

 addr geq(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(a >= b));
  OPDONE;
 }

 addr leq(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(a <= b));
  OPDONE;
 }

 addr land(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(cast(bool, a) && cast(bool, b)));
  OPDONE;
 }

 addr lor(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(fpu(cast(bool, a) || cast(bool, b)));
  OPDONE;
 }

 addr lnot(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  fpu a = memory::pop();
  push(fpu(!cast(bool, a)));
  OPDONE;
 }

 /* bit */
 addr band(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a & b);
  OPDONE;
 }

 addr bor(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a | b);
  OPDONE;
 }

 addr bnot(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  fpu a = memory::pop();
  push(~a);
  OPDONE;
 }

 addr bshl(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a << cast(s32, b));
  OPDONE;
 }

 addr bshr(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a >> cast(s32, b));
  OPDONE;
 }

 /* module */
 addr call(fpu value) {
  return module::table[cast(s32, value)].function(value);
 }

 /* nop */
 addr nop(fpu value) {
  OPDONE;
 }
}