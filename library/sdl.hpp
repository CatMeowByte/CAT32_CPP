#pragma once

#include <SDL3/SDL.h>

#include "core/constant.hpp" // IWYU pragma: keep

namespace sdl {
 bool video(octo* data);
 bool audio(fpu* data);

 bool poll();
 void stop();

 // alias
 constexpr void (*delay)(u32) = SDL_Delay;
 constexpr u64 (*get_ticks)() = SDL_GetTicks;

 bool is_key_pressed(const str key_name);
 void flip();
}