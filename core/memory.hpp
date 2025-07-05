#pragma once

#include "core/constant.hpp" // IWYU pragma: keep
#include "module/interpreter.hpp" // IWYU pragma: keep

// memory
extern u32 memory[SYSTEM::MEMORY];
extern u32 slotter;
extern u32 stacker;

//b bytecode
extern u32 bytecode[SYSTEM::CODESIZE];
extern u32 writer;

// interpreter
extern hash_map<string, u32> symbols;

extern hash_map<string, Redirect> redirect;

extern vector<IndentFrame> indent_stack;
extern u8 indent_previous;
extern IndentType indent_type_pending;
extern u32 loop_start;

// executor
extern u32 counter;

namespace memory_management {
 void memory_reset();
 void bytecode_reset();
 void executor_reset();
}

// TODO:
// separate pointer, counter, and stacker for each app