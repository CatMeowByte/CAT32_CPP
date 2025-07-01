#include "core/memory.hpp"

u32 memory[SYSTEM::MEMORY];
u32 slotter = 0;
u32 stacker = SYSTEM::MEMORY;

u32 bytecode[SYSTEM::CODESIZE];
u32 writer;

u32 counter;