#pragma once

#include "spec/spec.hpp"

bool sdl_init();
void sdl_shutdown();
bool sdl_poll();

void sdl_delay(int ms);
u32 sdl_get_ticks();

void sdl_flip(const u8* data);
