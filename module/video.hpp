#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace video {
 u8 pixel(s32 x, s32 y);
 void pixel(s32 x, s32 y, u8 color);
 void line(s32 ax, s32 ay, s32 bx, s32 by, u8 color);
 void rect(s32 x, s32 y, s32 width, s32 height, u8 color, bool fill);
 void text(s32 x, s32 y, string text, u8 color, u8 background = 0);

 void clear(u8 color);
 void flip();

 void module_register();
}