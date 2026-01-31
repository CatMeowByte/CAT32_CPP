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

namespace AUDIO {
 constexpr u16 RATE = 8000;
 constexpr u16 BUFFER = 512;
 constexpr u8 CHANNEL = 4;
}

namespace TICK {
 constexpr u8 UPDATE = 30;
 constexpr u8 DRAW = 15;
}

constexpr fpu SENTINEL = fpu(0x80000000, true); //  compile-time constant