#include "core/constant.hpp"
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
 addr push(elem value) {
  BAIL_IF_STACK_OVERFLOW
  memory[--stacker] = value;
  return SENTINEL;
 }

 addr pop(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  return memory[stacker++];
 }

 /* memory */
 addr takefrom(elem value) {
  BAIL_IF_STACK_OVERFLOW
  memory[--stacker] = memory[value];
  return SENTINEL;
 }

 addr storeto(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  memory[value] = memory[stacker++];
  return SENTINEL;
 }

 addr peek(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  addr address = fpu::unpack(pop(0));
  push(memory[address]);
  return SENTINEL;
 }

 addr poke(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  elem a = pop(0);
  addr address = fpu::unpack(pop(0));
  memory[address] = a;
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
  BAIL_UNLESS_STACK_ATLEAST(1)
  elem check = pop(0);
  if (check == 0) {return value;}
  return SENTINEL;
 }

 addr junz(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  elem check = pop(0);
  if (check != 0) {return value;}
  return SENTINEL;
 }

 /* math */
 addr add(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  elem b = pop(0);
  elem a = pop(0);
  push(a + b);
  return SENTINEL;
 }

 addr sub(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  elem b = pop(0);
  elem a = pop(0);
  push(a - b);
  return SENTINEL;
 }

 addr mul(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  s64 b = cast(s32, pop(0));
  s64 a = cast(s32, pop(0));
  push(cast(elem, fpu::unpack_wide(a * b)));
  return SENTINEL;
 }

 addr div(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  s64 b = cast(s32, pop(0));
  s64 a = cast(s32, pop(0));
  if (b == 0) {
   push(SENTINEL);
   return SENTINEL;
  }
  push(cast(elem, fpu::pack_wide(a) / b));
  return SENTINEL;
 }

 addr neg(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  elem a = pop(0);
  push(-a);
  return SENTINEL;
 }

 /* logic */
 addr eq(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  elem b = pop(0);
  elem a = pop(0);
  push(fpu::pack(a == b));
  return SENTINEL;
 }

 addr neq(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  elem b = pop(0);
  elem a = pop(0);
  push(fpu::pack(a != b));
  return SENTINEL;
 }

 addr gt(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  elem b = pop(0);
  elem a = pop(0);
  push(fpu::pack(a > b));
  return SENTINEL;
 }

 addr lt(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  elem b = pop(0);
  elem a = pop(0);
  push(fpu::pack(a < b));
  return SENTINEL;
 }

 addr geq(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  elem b = pop(0);
  elem a = pop(0);
  push(fpu::pack(a >= b));
  return SENTINEL;
 }

 addr leq(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  elem b = pop(0);
  elem a = pop(0);
  push(fpu::pack(a <= b));
  return SENTINEL;
 }

 addr land(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  elem b = pop(0);
  elem a = pop(0);
  push(fpu::pack(cast(bool, a) && cast(bool, b)));
  return SENTINEL;
 }

 addr lor(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  elem b = pop(0);
  elem a = pop(0);
  push(fpu::pack(cast(bool, a) || cast(bool, b)));
  return SENTINEL;
 }

 addr lnot(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  elem a = pop(0);
  push(fpu::pack(!cast(bool, a)));
  return SENTINEL;
 }

 /* bit */
 addr band(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  elem b = pop(0);
  elem a = pop(0);
  push(a & b);
  return SENTINEL;
 }

 addr bor(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  elem b = pop(0);
  elem a = pop(0);
  push(a | b);
  return SENTINEL;
 }

 addr bnot(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  elem a = pop(0);
  push(~a);
  return SENTINEL;
 }

 addr bshl(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  elem b = pop(0);
  elem a = pop(0);
  push(a << fpu::unpack(b));
  return SENTINEL;
 }

 addr bshr(elem value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  elem b = pop(0);
  elem a = pop(0);
  push(a >> fpu::unpack(b));
  return SENTINEL;
 }

 /* builtin */
 addr call(elem value) {
  return builtin::table[value].function(value);
 }

 /* nop */
 addr nop(elem value) {
  return SENTINEL;
 }
}