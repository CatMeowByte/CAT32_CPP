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
 // FIXME: likely need to be a special internal function to manage memory since the return type is ??
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

 // TODO: when adding region system make these works with bytes not fpu elem
 // addr peek(elem value) {
 //  BAIL_UNLESS_STACK_ATLEAST(1)
 //  addr address = fpu::unpack(pop(0));
 //  push(memory[address]);
 //  return SENTINEL;
 // }

 // addr poke(elem value) {
 //  BAIL_UNLESS_STACK_ATLEAST(2)
 //  elem a = pop(0);
 //  addr address = fpu::unpack(pop(0));
 //  memory[address] = a;
 //  return SENTINEL;
 // }

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

 addr jumz(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  fpu check = pop(0);
  if (!check) {return value;}
  return SENTINEL;
 }

 addr junz(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  fpu check = pop(0);
  if (check) {return value;}
  return SENTINEL;
 }

 /* math */
 addr add(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  push(a + b);
  return SENTINEL;
 }

 addr sub(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  push(a - b);
  return SENTINEL;
 }

 addr mul(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  push(a * b);
  return SENTINEL;
 }

 addr div(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  if (!b) {
   push(fpu(0));
   return SENTINEL;
  }
  push(a / b);
  return SENTINEL;
 }

 addr neg(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  fpu a = pop(0);
  push(-a);
  return SENTINEL;
 }

 /* logic */
 addr eq(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  push(fpu(a == b));
  return SENTINEL;
 }

 addr neq(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  push(fpu(a != b));
  return SENTINEL;
 }

 addr gt(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  push(fpu(a > b));
  return SENTINEL;
 }

 addr lt(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  push(fpu(a < b));
  return SENTINEL;
 }

 addr geq(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  push(fpu(a >= b));
  return SENTINEL;
 }

 addr leq(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  push(fpu(a <= b));
  return SENTINEL;
 }

 addr land(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  push(fpu(cast(bool, a) && cast(bool, b)));
  return SENTINEL;
 }

 addr lor(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  push(fpu(cast(bool, a) || cast(bool, b)));
  return SENTINEL;
 }

 addr lnot(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  fpu a = pop(0);
  push(fpu(!cast(bool, a)));
  return SENTINEL;
 }

 /* bit */
 addr band(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  push(a & b);
  return SENTINEL;
 }

 addr bor(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  push(a | b);
  return SENTINEL;
 }

 addr bnot(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  fpu a = pop(0);
  push(~a);
  return SENTINEL;
 }

 addr bshl(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  push(a << cast(s32, b));
  return SENTINEL;
 }

 addr bshr(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = pop(0);
  fpu a = pop(0);
  push(a >> cast(s32, b));
  return SENTINEL;
 }

 /* builtin */
 addr call(fpu value) {
  return builtin::table[cast(s32, value)].function(value);
 }

 /* nop */
 addr nop(fpu value) {
  return SENTINEL;
 }
}