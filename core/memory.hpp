#pragma once

#include "core/constant.hpp" // IWYU pragma: keep
#include "module/interpreter.hpp" // IWYU pragma: keep
#include <vector>

// memory
extern elem memory[SYSTEM::MEMORY];
extern u32 slotter;
extern addr stacker;

//b bytecode
extern elem bytecode[SYSTEM::CODESIZE];
extern addr writer;
extern vector<addr> framer;

// interpreter

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
  addr address = SENTINEL;
  Type type = Type::Number;
 };

 extern vector<Data> table;

 s32 get_index_reverse(const string& name);
 bool exist(const string& name);
 Data& get(const string& name);
}

extern hash_map<string, Redirect> redirect;

extern vector<IndentFrame> indent_stack;
extern u8 indent_previous;
extern IndentType indent_type_pending;
extern addr header_start;

// executor
extern addr counter;
extern u32 sleeper;

namespace memory_management {
 void memory_reset();
 void bytecode_reset();
 void executor_reset();
}

namespace fpu {
 inline s32 pack(s32 number) {return number << SYSTEM::FIXED_POINT_WIDTH;}
 inline s32 unpack(s32 fixed_point) {return fixed_point >> SYSTEM::FIXED_POINT_WIDTH;}

 inline s64 pack_wide(s64 number) {return number << SYSTEM::FIXED_POINT_WIDTH;}
 inline s64 unpack_wide(s64 fixed_point) {return fixed_point >> SYSTEM::FIXED_POINT_WIDTH;}

 inline double scale(double number) {return number * (1 << SYSTEM::FIXED_POINT_WIDTH);}
 inline double unscale(s32 fixed_point) {return cast(double, fixed_point) / (1 << SYSTEM::FIXED_POINT_WIDTH);}
}

// TODO:
// separate pointer, counter, and stacker for each app