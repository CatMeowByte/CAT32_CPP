#pragma once

#include "core/constant.hpp" // IWYU pragma: keep
#include "module/opcode.hpp" // IWYU pragma: keep

// memory
extern u32 memory[SYSTEM::MEMORY];
extern u32 slotter;
extern u32 stacker;

//b bytecode
extern u32 bytecode[SYSTEM::CODESIZE];
extern u32 writer;

// interpreter
extern hash_map<string, u32> symbols;

struct Redirect {u32 address = SENTINEL; vector<u32> pending;};
extern hash_map<string, Redirect> redirect;

extern u8 indent_previous;
extern vector<u32> indent_stack;

// executor
extern u32 counter;

namespace memory_management {
 void memory_reset();
 void bytecode_reset();
 void executor_reset();
}

// TODO:
// separate pointer, counter, and stacker for each app