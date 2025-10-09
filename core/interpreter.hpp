#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

struct TokenLine {
 u8 indent; // = 0;
 vector<string> tokens;
};

struct Redirect {addr address = SENTINEL; vector<addr> pending;}; // TODO: delete goto jump likely incompatible with scope

namespace interpreter {
 TokenLine tokenize(const string& text);
 void compile(const TokenLine& tokens);
 void step();
}

namespace module {
 struct Module {
  string name;
  fnp function;
 };

 extern vector<Module> table;
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