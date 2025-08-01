#include "core/memory.hpp"
#include <cstring>

// memory
elem memory[SYSTEM::MEMORY];
u32 slotter = 0;
addr stacker = SYSTEM::MEMORY;

//b bytecode
elem bytecode[SYSTEM::CODESIZE];
addr writer = 0;
vector<addr> framer;

// executor
addr counter = 0;
u32 sleeper = 0;

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
  framer.clear();

  // interpreter
  symbols.clear();
  redirect.clear();
  indent_stack.clear();
  indent_previous = 0;
  indent_type_pending = IndentType::UNKNOWN;
  header_start = 0;
 }

 void executor_reset() {
  counter = 0;
  sleeper = 0;
 }
}