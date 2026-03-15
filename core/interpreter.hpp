#pragma once

#include "core/constant.hpp" // IWYU pragma: keep
#include "core/define.hpp"


namespace interpreter {

 vector<vector<string>> tokenize(const string& line);
 void compile(const vector<vector<string>>& line_tokens);
 void step();
 void reset();
}

namespace symbol {
 enum class Type : u8 {
  Number,
  Constant,
  Stripe,
  Function,
 };

 struct Data {
  string name;
  address_logic address;
  Type type;
  fpu value;
  u8 args_count;
  vector<fpu> args_default;
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
  address_logic jump_operand;
  address_logic header_start;
  u32 symbol_start;
  vector<address_logic> break_unpatched;
  vector<u16> space;
 };

 extern vector<Frame> stack;
 extern u8 previous;

 extern address_logic last_jump_operand;
 extern address_logic last_line_start; // for while loop
 extern Type last_line_scope_set;
}