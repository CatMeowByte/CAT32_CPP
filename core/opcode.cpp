#include "core/constant.hpp"
#include "core/module.hpp"
#include "core/memory.hpp"
#include "core/opcode.hpp"

namespace opcode {
 static const hash_map<string, u8> opcode_table = {
  #define OP(hex, name) {#name, hex},
  #define OPA(hex, name) {#name, hex},
  #define OPV(hex, name) {#name, hex},
  OPCODES
  #undef OP
  #undef OPA
  #undef OPV
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
   #define OPA(hex, name) case hex: return #name;
   #define OPV(hex, name) case hex: return #name;
   OPCODES
   #undef OP
   #undef OPA
   #undef OPV
  }
  return "UNKNOWN";
 }
}

namespace op_call {
 /* stack */
 OPCODE_VALUE(push, {
  memory::push(operand);
 })

 /* memory */
 // all memory opcodes use slot index
 OPCODE_ADDRESS(takefrom, {
  memory::push(active::logic->code_fpu[operand]);
 })

 OPCODE_ADDRESS(storeto, {
  fpu value = memory::pop();
  active::logic->code_fpu[operand] = value;
 })

 OPCODE_ADDRESS(stampto, {
  slot_logic source = memory::pop().i();
  slot_logic destination = operand;
  u16 length = active::logic->code_fpu[source].i() + 1;
  u16 capacity = active::logic->code_fpu[destination - 1].i();
  memcpy(active::logic->code_fpu + destination, active::logic->code_fpu + source, min(length, capacity) * sizeof(fpu));
 })

 OPCODE(get, {
  slot_logic slot = memory::pop().i();
  memory::push(active::logic->code_fpu[slot]);
 })

 OPCODE(set, {
  fpu value = memory::pop();
  slot_logic slot = memory::pop().i();
  active::logic->code_fpu[slot] = value;
 })

 /* counter */
 OPCODE_ADDRESS(subgo, {
  if (cast(u32, active::logic->framer.i()) >= memory::vm::process::p0::logic::frames_length) {return active::logic->counter.a() + 3;}
  active::logic->frames[(++active::logic->framer).i()] = active::logic->counter;
  return operand;
 })

 OPCODE(subret, {
  if (active::logic->framer == fpu(0)) {return active::logic->writer.a() + 1;}
  return active::logic->frames[active::logic->framer--.i()].a() + 3;
 })

 OPCODE_ADDRESS(jump, {
  if (operand == FARLAND) {return active::logic->writer.a() + 1;}
  return operand;
 })

 OPCODE_ADDRESS(jumz, {
  fpu check = memory::pop();
  if (!check) {return operand;}
 })

 OPCODE_ADDRESS(junz, {
  fpu check = memory::pop();
  if (check) {return operand;}
 })

 /* math */
 OPCODE(add, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(a + b);
 })

 OPCODE(sub, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(a - b);
 })

 OPCODE(mul, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(a * b);
 })

 OPCODE(div, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(b ? a / b : SENTINEL);
 })

 OPCODE(mod, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(b ? a - fpu(floor(a / b)) * b : SENTINEL);
 })

 OPCODE(neg, {
  fpu a = memory::pop();
  memory::push(-a);
 })

 /* logic */
 OPCODE(eq, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(a == b);
 })

 OPCODE(neq, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(a != b);
 })

 OPCODE(gt, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(a > b);
 })

 OPCODE(lt, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(a < b);
 })

 OPCODE(geq, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(a >= b);
 })

 OPCODE(leq, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(a <= b);
 })

 OPCODE(land, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(a && b);
 })

 OPCODE(lor, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(a || b);
 })

 OPCODE(lnot, {
  fpu a = memory::pop();
  memory::push(!a);
 })

 /* bit */
 OPCODE(band, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(a & b);
 })

 OPCODE(bor, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(a | b);
 })

 OPCODE(bnot, {
  fpu a = memory::pop();
  memory::push(~a);
 })

 OPCODE(bshl, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(a << cast(s32, b));
 })

 OPCODE(bshr, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(a >> cast(s32, b));
 })

 /* marker */
 OPCODE(prime, {
  active::logic->stacker = active::logic->slotter;
 })

 /* module */
 OPCODE_VALUE(call, {
  module::table[operand.r()].function();
 })

 /* nop */
 OPCODE(nop, {})
}