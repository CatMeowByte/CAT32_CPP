#pragma once

#include <SDL3/SDL.h>

#include "core/constant.hpp" // IWYU pragma: keep

namespace sdl {
 // initializer
 bool audio(fpu* data);
 bool button(fpu* data);
 bool video(octo* data);

 // updater
 namespace update {
  void button();
  void gyroscope(fpu* data);
  void window();
 }

 bool poll();
 void stop();

 // alias
 constexpr void (*delay)(u32) = SDL_Delay;
 constexpr u64 (*get_ticks)() = SDL_GetTicks;

 void flip();
}