#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

enum Opcode : u8 {
 NOP = 0x00,
 PUSH = 0x11,
 POP = 0x12,
 CLEAR = 0xA0,
 PIXEL = 0xA1,
 FLIP = 0xAF,
};

namespace op {
 void push(u8 value);
 u8 pop();

 void clear();
 void pixel();
 void rect();
 void flip();
}