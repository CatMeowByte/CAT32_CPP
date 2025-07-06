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

// interpreter
extern hash_map<string, addr> symbols;

extern hash_map<string, Redirect> redirect;

extern vector<IndentFrame> indent_stack;
extern u8 indent_previous;
extern IndentType indent_type_pending;
extern addr loop_start;

// executor
extern addr counter;

namespace memory_management {
 void memory_reset();
 void bytecode_reset();
 void executor_reset();
}

// TODO:
// separate pointer, counter, and stacker for each app