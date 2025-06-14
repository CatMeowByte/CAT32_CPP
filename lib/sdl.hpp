#pragma once

#include "spec/spec.hpp"

namespace sdl {
 bool init();
 void shutdown();
 bool poll();

 void delay(int ms);
 u32 get_ticks();

 void flip(const u8* data);
}