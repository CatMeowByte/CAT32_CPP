#pragma once

#include "spec/spec.hpp"
#include <uchar.h>

u8 pixel(s16 x, s16 y);
void pixel(s16 x, s16 y, u8 color);
void line(s16 ax, s16 ay, s16 bx, s16 by, u8 color);
void rect(s16 x, s16 y, s16 width, s16 height, u8 color, bool fill);
void text(s16 x, s16 y, const char* str, u8 color, u8 background = 0);

void clear(u8 color);
void flip();
