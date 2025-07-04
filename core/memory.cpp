#include "core/memory.hpp"
#include "core/constant.hpp"
#include <cstring>

// memory
u32 memory[SYSTEM::MEMORY];
u32 slotter = 0;
u32 stacker = SYSTEM::MEMORY;

//b bytecode
u32 bytecode[SYSTEM::CODESIZE];
u32 writer = 0;

// executor
u32 counter = 0;

// TODO:
// possibly move the variable definition to the appropriate module/file that control it

namespace memory_management {
 void memory_reset() {
  memset(memory, 0, sizeof(memory));
  slotter = 0;
  stacker = SYSTEM::MEMORY;
 }

 void bytecode_reset() {
  memset(bytecode, 0, sizeof(bytecode));
  writer = 0;

  // interpreter
  symbols.clear();
  redirect.clear();
  indent_previous = 0;
  indent_stack.clear();
 }

 void executor_reset() {
  counter = 0;
 }
}