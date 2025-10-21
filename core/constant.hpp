#pragma once

#include "core/define.hpp"

namespace SYSTEM {
 constexpr str NAME = "CAT-32";
 constexpr str CODENAME = "CAT32";
 constexpr u32 MEMORY = 1024 * 128; // 128 KiB
 constexpr u16 CODESIZE = 1024 * 4; // 4 KiB
}

namespace VIDEO {
 constexpr u16 WIDTH = 120;
 constexpr u16 HEIGHT = 160;
 constexpr u8 SCALE = 3;
}

namespace TICK {
 constexpr u8 UPDATE = 15;
 constexpr u8 DRAW = 30;
}

constexpr fpu SENTINEL = fpu(-2147483648.0); // minimum negative
constexpr fpu SIGNATURE = fpu(0xFACADE32); // placeholder byte data