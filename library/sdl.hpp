#pragma once

#include <SDL3/SDL_scancode.h>

#include "core/constant.hpp" // IWYU pragma: keep

namespace sdl {
 bool init();
 void shutdown();
 bool poll();

 void delay(const u32 ms);
 u64 get_ticks();

 bool is_key_pressed(const str key_name);

 void flip(const u8* data);
}