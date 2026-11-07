#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"

// TODO:
// this is so small, probably will be turned into unified input module

namespace button {
 using namespace memory::vm::global;

 namespace wrap {
  OPCODE(button, {
   u8 index = memory::pop();

   fpu state = hardware_io::button_fpu[index % BUTTON::COUNT];
   memory::push(state);
  })
 }

 MODULE(
  module::add("", "button", wrap::button, 1);
 )
}