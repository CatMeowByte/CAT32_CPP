#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

extern u32 memory[SYSTEM::MEMORY];
extern u32 slotter;
extern u32 stacker;

extern u32 bytecode[SYSTEM::CODESIZE];
extern u32 writer;

extern u32 counter;

// TODO:
// separate pointer, counter, and stacker for each app