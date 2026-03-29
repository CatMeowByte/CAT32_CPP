#include "core/constant.hpp"
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

  active::logic->counter = fpu::raw(result);
 }

 void reset() {
   symbol::table.clear();

   scope::stack.clear();

   scope::previous::indent = 0;
   scope::previous::type = scope::Type::Generic;
   scope::previous::line = FARLAND;
   scope::previous::skip_operand = FARLAND;

   // base scope frame
   scope::Frame base = {};
   base.type = scope::Type::Generic;
   scope::stack.push_back(base);
 }
}