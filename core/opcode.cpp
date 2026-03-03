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
  using namespace memory::vm::process::app::ram_local;
  field[(--stacker).i()] = operand;
 })

 /* memory */
 OPCODE_ADDRESS(takefrom, {
  using namespace memory::vm::process::app;
  using namespace ram_local;
  field[(--stacker).i()] = ram_local_fpu[operand];
 })

 OPCODE_ADDRESS(storeto, {
  using namespace memory::vm::process::app;
  fpu value = memory::pop();
  ram_local_fpu[operand] = value;
 })

 OPCODE(get, {
  using namespace memory::vm::process::app;
  code_address address = memory::pop();
  memory::push(ram_local_fpu[address]);
 })

 OPCODE(set, {
  using namespace memory::vm::process::app;
  fpu value = memory::pop();
  code_address address = memory::pop();
  ram_local_fpu[address] = value;
 })

 /* counter */
 OPCODE_ADDRESS(subgo, {
  using namespace memory::vm::process::app::ram_local;
  if (cast(u32, framer.i()) >= frames_length) {return counter.a() + 3;}
  frames[(framer++).i()] = counter;
  return operand;
 })

 OPCODE(subret, {
  using namespace memory::vm;
  using namespace ram_global::constant;
  using namespace process::app::ram_local;
  if (framer == zero) {return writer.a();} // end kernel event loop
  return frames[(--framer).i()].a() + 3;
 })

 OPCODE_ADDRESS(jump, {
  if (operand == 0xFFFF) {return memory::vm::process::app::ram_local::writer.a();} // end of code_address
  return operand;
 })

 OPCODE_ADDRESS(jumz, {
  using namespace memory::vm::process::app::ram_local;
  fpu check = memory::pop();
  if (!check) {return operand;}
 })

 OPCODE_ADDRESS(junz, {
  using namespace memory::vm::process::app::ram_local;
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
  memory::push(b ? a / b : memory::vm::ram_global::constant::sentinel);
 })

 OPCODE(mod, {
  fpu b = memory::pop();
  fpu a = memory::pop();
  memory::push(b ? a - fpu(floor(a / b)) * b : memory::vm::ram_global::constant::sentinel);
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
  using namespace memory::vm::process::app::ram_local;
  stacker = field_length;
 })

 /* module */
 OPCODE_VALUE(call, {
  module::table[operand.r()].function();
 })

 /* nop */
 OPCODE(nop, {})
}