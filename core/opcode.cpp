#include "core/module.hpp"
#include "core/memory.hpp"
#include "core/opcode.hpp"

namespace opcode {
 struct OpcodeInfo {u8 id; u8 args;};
 static const hash_map<string, OpcodeInfo> opcode_table = {
  #define OPI(hex, name)
  #define OPC(hex, name, args) {#name, {op::name, args}},
  OPCODES
  #undef OPI
  #undef OPC
 };

 u8 get(string cmd) {
  return opcode_table.count(cmd) ? opcode_table.at(cmd).id : op::nop;
 }

 u8 args_count(string cmd) {
  return opcode_table.count(cmd) ? opcode_table.at(cmd).args : 0;
 }

 bool exist(string cmd) {
  return opcode_table.count(cmd);
 }

 string name(u8 value) {
  switch (value) {
   #define OPI(hex, name) case hex: return #name;
   #define OPC(hex, name, args) case hex: return #name;
   OPCODES
   #undef OPI
   #undef OPC
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

 /* memory */
 addr takefrom(fpu value) {
  BAIL_IF_STACK_OVERFLOW
  using namespace memory::vm::process::app;
  using namespace ram_local;
  field[--stacker] = ram_local_fpu[value];
  OPDONE;
 }

 addr storeto(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  using namespace memory::vm::process::app;
  ram_local_fpu[value] = memory::pop();
  OPDONE;
 }

 addr get(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  using namespace memory::vm::process::app;
  addr address = memory::pop();
  push(ram_local_fpu[address]);
  OPDONE;
 }

  addr set(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(2)
   using namespace memory::vm::process::app;
   fpu a = memory::pop();
   addr address = memory::pop();
   ram_local_fpu[address] = a;
   OPDONE;
  }

  addr peek8(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(1)
   s32 address_signed = memory::pop();
   bool is_global = address_signed < 0;
   addr address = is_global ? -address_signed : address_signed;
   octo* region = is_global ? memory::vm::ram_global_octo : memory::vm::process::app::ram_local_octo;
   push(fpu(region[address]));
   OPDONE;
  }

  addr poke8(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(2)
   fpu val = memory::pop();
   s32 address_signed = memory::pop();
   bool is_global = address_signed < 0;
   addr address = is_global ? -address_signed : address_signed;
   octo* region = is_global ? memory::vm::ram_global_octo : memory::vm::process::app::ram_local_octo;
   region[address] = cast(octo, val);
   OPDONE;
  }

  addr peek32(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(1)
   s32 address_signed = memory::pop();
   bool is_global = address_signed < 0;
   addr address = is_global ? -address_signed : address_signed;
   octo* region = is_global ? memory::vm::ram_global_octo : memory::vm::process::app::ram_local_octo;
   s32 raw = memory::unaligned_32_read(region + address);
   push(fpu(raw, true));
   OPDONE;
  }

  addr poke32(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(2)
   fpu val = memory::pop();
   s32 address_signed = memory::pop();
   bool is_global = address_signed < 0;
   addr address = is_global ? -address_signed : address_signed;
   octo* region = is_global ? memory::vm::ram_global_octo : memory::vm::process::app::ram_local_octo;
   memory::unaligned_32_write(region + address, val.value);
   OPDONE;
  }

 /* counter */
 addr subgo(fpu value) {
  using namespace memory::vm::process::app::ram_local;
  if (framer >= fpu(frames_length)) {OPDONE;}
  frames[framer++] = counter;
  return value;
 }

 addr subret(fpu value) {
  using namespace memory::vm::process::app::ram_local;
  if (framer == fpu(0)) {OPDONE;}
  addr address = frames[--framer];
  return address + 5;
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
  push(b ? a / b : fpu(0));
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
  push(a == b);
  OPDONE;
 }

 addr neq(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a != b);
  OPDONE;
 }

 addr gt(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a > b);
  OPDONE;
 }

 addr lt(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a < b);
  OPDONE;
 }

 addr geq(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a >= b);
  OPDONE;
 }

 addr leq(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a <= b);
  OPDONE;
 }

 addr land(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a && b);
  OPDONE;
 }

 addr lor(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(2)
  fpu b = memory::pop();
  fpu a = memory::pop();
  push(a || b);
  OPDONE;
 }

 addr lnot(fpu value) {
  BAIL_UNLESS_STACK_ATLEAST(1)
  fpu a = memory::pop();
  push(!a);
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