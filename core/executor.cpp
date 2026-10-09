#include "core/interpreter.hpp"
#include "core/memory.hpp"
#include "core/opcode.hpp"

namespace interpreter {
 void step() {
  if (active::logic->counter > active::logic->writer) {return;}

  octo opcode = active::logic->code_octo[active::logic->counter.a()];
  address_logic result;

  switch (opcode) {
   #define OP(hex, name) case op::name: result = op_call::name(); break;
   #define OPA(hex, name) case op::name: result = op_call::name(); break;
   #define OPV(hex, name) case op::name: result = op_call::name(); break;
   OPCODES
   #undef OP
   #undef OPA
   #undef OPV
  }

  active::logic->counter = result;
 }
}