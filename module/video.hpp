#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace video {
 void clear(u8 color);
 u8 pixel(s32 x, s32 y, s8 color);
 void line(s32 ax, s32 ay, s32 bx, s32 by, u8 color);
 void rect(s32 x, s32 y, s32 width, s32 height, u8 color, bool fill);
 void circle(s32 x, s32 y, s32 radius, u8 color, bool fill);
 void text(s32 x, s32 y, string text, u8 color, u8 background = 0);
}