#pragma once

#include "core/constant.hpp" // IWYU pragma: keep
#include "core/opcode.hpp" // IWYU pragma: keep

namespace interpreter {
 static const hash_set<string> keywords_declaration = {"var", "con", "str", "func", "use"};
 static const hash_set<string> keywords_control = {"if", "elif", "else", "while", "break", "continue", "return"};

 // token tagged with this symbol can only be generated internally
 namespace tag {
  constexpr str offset = "+>"; // var[n]
  constexpr str callable_args = "=#"; // func(n)
 }

 vector<vector<string>> tokenize(const string& line);
 void reset();
 void compile(const vector<vector<string>>& line_tokens);

 void step();
}

namespace symbol {
 enum class Type : u8 {Number, Constant, Stripe, Function};

 struct Data {
  Type type;
  string name;
  union {
   struct {slot_logic slot;} variable;
   struct {fpu value;} constant;
   struct {slot_logic slot; u16 capacity;} stripe;
   struct {address_logic address; u8 args_count;} function;
  };
  vector<fpu> args_default;
 };

 s32 get_index_reverse(const string& name);
 bool exist(const string& name);
 Data& get(const string& name);
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

 enum class Order : u8 {
  None = 0,
  Strict = 1,
  Free = 2,
 };

 // form, opcode, precedence, associative
 #define SORTED_OPERATORS \
  OP("==", op::eq, Precedence::Compare, Order::Strict) \
  OP("!=", op::neq, Precedence::Compare, Order::Strict) \
  OP("<=", op::leq, Precedence::Compare, Order::Strict) \
  OP(">=", op::geq, Precedence::Compare, Order::Strict) \
  OP("&&", op::band, Precedence::Logic, Order::Free) \
  OP("||", op::bor, Precedence::Logic, Order::Free) \
  OP("<<", op::bshl, Precedence::Shift, Order::Strict) \
  OP(">>", op::bshr, Precedence::Shift, Order::Strict) \
  OP("+", op::add, Precedence::Add, Order::Free) \
  OP("-", op::sub, Precedence::Add, Order::Strict) \
  OP("*", op::mul, Precedence::Multiply, Order::Free) \
  OP("/", op::div, Precedence::Multiply, Order::Strict) \
  OP("%", op::mod, Precedence::Multiply, Order::Strict) \
  OP("<", op::lt, Precedence::Compare, Order::Strict) \
  OP(">", op::gt, Precedence::Compare, Order::Strict) \
  OP("&", op::land, Precedence::Logic, Order::Free) \
  OP("|", op::lor, Precedence::Logic, Order::Free) \
  OP("!", op::lnot, Precedence::Unary, Order::None) \
  OP("~", op::bnot, Precedence::Unary, Order::None) \

 static const hash_set<string> operations = {
  #define OP(sym, code, prec, assoc) sym,
  SORTED_OPERATORS
  #undef OP
 };

 static const hash_map<string, u8> opcodes = {
  #define OP(sym, code, prec, assoc) {sym, code},
  SORTED_OPERATORS
  #undef OP
 };

 static const hash_map<string, Precedence> precedences = {
  #define OP(sym, code, prec, assoc) {sym, prec},
  SORTED_OPERATORS
  #undef OP
 };

 static const hash_map<string, Order> orders = {
  #define OP(sym, code, prec, assoc) {sym, assoc},
  SORTED_OPERATORS
  #undef OP
 };

 #undef SORTED_OPERATORS
}