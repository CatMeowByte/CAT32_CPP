#include "core/memory.hpp"

// memory
elem memory[SYSTEM::MEMORY];
u32 slotter = 0;
addr stacker = SYSTEM::MEMORY;

//b bytecode
elem bytecode[SYSTEM::CODESIZE];
addr writer = 0;
vector<addr> framer;

// global
namespace builtin {
 vector<Builtin> table;
}

// executor
addr counter = 0;
u32 sleeper = 0;

// TODO:
// possibly move the variable definition to the appropriate module/file that control it
// or not, why dont keep things organized by just clump it in one predictable searchable place
// just define it here

namespace symbol {
 vector<Data> table;

 s32 get_index_reverse(const string& name) {
  for (u32 i = table.size(); i-- > 0;) {
   if (symbol::table[i].name == name) {return i;}
  }
  return -1;
 }

 bool exist(const string& name) {
  return get_index_reverse(name) >= 0;
 }

 Data& get(const string& name) {
  return table[get_index_reverse(name)];
 }
}

namespace scope {
 vector<Frame> stack;
 u8 previous = 0;

 addr last_jump_operand = SENTINEL;
 addr last_line_start = SENTINEL;
 Type last_line_scope_set = Type::Generic;
}

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

  symbol::table.clear();

  redirect.clear();

  scope::stack.clear();
  scope::previous = 0;
  scope::last_jump_operand = SENTINEL;
  scope::last_line_start = SENTINEL;
  scope::last_line_scope_set = scope::Type::Generic;
 }

 void executor_reset() {
  counter = 0;
  sleeper = 0;
 }
}