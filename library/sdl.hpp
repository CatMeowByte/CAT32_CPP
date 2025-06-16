#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace sdl {
 bool init();
 void shutdown();
 bool poll();

 void delay(u32 ms);
 u32 get_ticks();

 void flip(const u8* data);
}