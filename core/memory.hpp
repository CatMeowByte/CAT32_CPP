#pragma once

#include "core/constant.hpp" // IWYU pragma: keep
#include "module/interpreter.hpp" // IWYU pragma: keep

// memory
extern elem memory[SYSTEM::MEMORY];
extern u32 slotter;
extern addr stacker;

//b bytecode
extern elem bytecode[SYSTEM::CODESIZE];
extern addr writer;
extern vector<addr> framer;

// interpreter
enum SymbolType {
  NUMBER,
  STRING,
  STRIPE,
  FUNCTION,
};

struct SymbolData {
  addr address;
  SymbolType type;
  // s32 attribute;
};

extern hash_map<string, SymbolData> symbols;

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

 inline float scale(float number) {return number * (1 << SYSTEM::FIXED_POINT_WIDTH);}
 inline float unscale(s32 fixed_point) {return cast(float, fixed_point) / (1 << SYSTEM::FIXED_POINT_WIDTH);}
}

// TODO:
// separate pointer, counter, and stacker for each app