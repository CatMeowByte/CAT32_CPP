#pragma once

#include "core/define.hpp"

namespace SYSTEM {
 constexpr str NAME = "CAT-32";
 constexpr str CODENAME = "CAT32";
 constexpr u16 MEMORY = 1024 * 8; // 8 KiB
 constexpr u16 CODESIZE = 1024 * 4; // 4 KiB
 constexpr u8 FIXED_POINT_WIDTH = 16; // 16.16
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

constexpr elem SENTINEL = 0x80000000; // minimum negative