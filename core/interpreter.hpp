#pragma once

#include "core/constant.hpp" // IWYU pragma: keep
#include "core/opcode.hpp"

namespace interpreter {
 static const hash_set<string> keywords_declaration = {"var", "con", "str", "func", "use"};
 static const hash_set<string> keywords_control = {"if", "else", "while", "break", "continue", "return"};

 // token tagged with this symbol can only be generated internally
 namespace tag {
  constexpr str offset = "+>"; // v[n]
  constexpr str callable_args = "=#"; // foo(n)
 }

 vector<vector<string>> tokenize(const string& line);
 void compile(const vector<vector<string>>& line_tokens);
 void step();
 void reset();
}

namespace symbol {
 enum class Type : u8 {Number, Constant, Stripe, Function};

 struct Data {
  Type type;
  string name;
  union {
   struct {slot_logic slot;} variable;
   struct {fpu value;} constant;
   struct {address_logic address; u8 args_count;} function;
  };
  vector<fpu> args_default;
 };

 extern vector<Data> table;

 s32 get_index_reverse(const string& name);
 bool exist(const string& name);
 Data& get(const string& name);
}

namespace scope {
 enum class Type : u8 {Generic, If, Else, While, Function};

 struct Frame {
  Type type;
  address_logic line;
  address_logic skip_operand;
  u32 symbol_boundary;
  vector<address_logic> break_operands;
  vector<u16> space;
 };

 extern vector<Frame> stack;

 namespace previous {
  extern u8 indent;
  extern Type type;
  extern address_logic line;
  extern address_logic skip_operand;
 }
}

namespace metic {
 namespace {
  enum class Precedence : u8 {
   Base = 0,
   Logic = 0,
   Compare = 1,
   Shift = 2,
   Add = 3,
   Multiply = 4,
   Offset = 5,
   Unary = 6,
  };
 }

 #define SORTED_OPERATORS \
  OP("==", op::eq, Precedence::Compare) \
  OP("!=", op::neq, Precedence::Compare) \
  OP("<=", op::leq, Precedence::Compare) \
  OP(">=", op::geq, Precedence::Compare) \
  OP("&&", op::band, Precedence::Logic) \
  OP("||", op::bor, Precedence::Logic) \
  OP("~~", op::bnot, Precedence::Unary) \
  OP("<<", op::bshl, Precedence::Shift) \
  OP(">>", op::bshr, Precedence::Shift) \
  OP("+", op::add, Precedence::Add) \
  OP("-", op::sub, Precedence::Add) \
  OP("*", op::mul, Precedence::Multiply) \
  OP("/", op::div, Precedence::Multiply) \
  OP("%", op::mod, Precedence::Multiply) \
  OP("<", op::lt, Precedence::Compare) \
  OP(">", op::gt, Precedence::Compare) \
  OP("&", op::land, Precedence::Logic) \
  OP("|", op::lor, Precedence::Logic) \
  OP("!", op::lnot, Precedence::Unary)

 static const hash_set<string> operations = {
  #define OP(sym, code, prec) sym,
  SORTED_OPERATORS
  #undef OP
 };

 static const hash_map<string, u8> opcodes = {
  #define OP(sym, code, prec) {sym, code},
  SORTED_OPERATORS
  #undef OP
 };

 static const hash_map<string, Precedence> precedences = {
  #define OP(sym, code, prec) {sym, prec},
  SORTED_OPERATORS
  #undef OP
 };

 #undef SORTED_OPERATORS
}