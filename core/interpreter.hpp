#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

struct TokenLine {
 u8 indent;
 vector<string> tokens;
};

namespace interpreter {
 TokenLine tokenize(const string& text);
 void compile(const TokenLine& tokens);
 void step();
}

namespace symbol {
 enum class Type : u8 {
  Number,
  String,
  Stripe,
  Function,
 };

 struct Data {
  string name;
  addr address;
  Type type;
 };

 extern vector<Data> table;

 s32 get_index_reverse(const string& name);
 bool exist(const string& name);
 Data& get(const string& name);
}

namespace scope {
 enum class Type : u8 {
  Generic,
  If,
  Else,
  While,
  Function,
 };

 struct Frame {
  Type type;
  addr jump_operand;
  addr header_start;
  addr stack_start;
  u32 symbol_start;
 };

 extern vector<Frame> stack;
 extern u8 previous;

 extern addr last_jump_operand;
 extern addr last_line_start; // for while loop
 extern Type last_line_scope_set;
}

struct Redirect {addr address = SENTINEL; vector<addr> pending;}; // TODO: delete goto jump likely incompatible with scope

extern hash_map<string, Redirect> redirect;