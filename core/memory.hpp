#pragma once

#include "core/constant.hpp"
#include "core/define.hpp"
#include "core/interpreter.hpp"

// memory
extern fpu memory[SYSTEM::MEMORY];
extern u32 slotter;
extern addr stacker;

// bytecode
extern elem bytecode[SYSTEM::CODESIZE];
extern addr writer;
extern vector<addr> framer;

// global
namespace builtin {
 struct Builtin {
  string name;
  fnp function;
 };

 extern vector<Builtin> table;
}

// symbol data are presistent in memory eventhough only used in compile time because of per line compile and console
// remember to clear the table before compiling app
namespace symbol {
 enum class Type : u8 {
  Number,
  String,
  Stripe,
  Function,
 };

 struct Data {
  string name;
  addr address; // = SENTINEL;
  Type type; // = Type::Number;
 };

 extern vector<Data> table;

 s32 get_index_reverse(const string& name);
 bool exist(const string& name);
 Data& get(const string& name);
}

extern hash_map<string, Redirect> redirect;

namespace scope {
 enum class Type : u8 {
  Generic,
  If,
  Else,
  While,
  Function,
 };

 struct Frame {
  addr jump_operand = SENTINEL;
  addr header_start = SENTINEL;
  Type type = Type::Generic;

  u32 symbol_start = 0;
  addr stack_start = SYSTEM::MEMORY;
 };

 extern vector<Frame> stack;
 extern u8 previous;

 extern addr last_jump_operand;
 extern addr last_line_start; // for while loop
 extern Type last_line_scope_set;
}

// executor
extern addr counter;
extern u32 sleeper;

namespace memory_management {
 void memory_reset();
 void bytecode_reset();
 void executor_reset();
}

// TODO:
// separate pointer, counter, and stacker for each app