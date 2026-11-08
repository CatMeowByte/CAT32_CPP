#pragma once

#include "core/define.hpp" // IWYU pragma: export

namespace SYSTEM {
 constexpr str NAME = "CAT-32";
 constexpr str CODENAME = "CAT32";
 constexpr u8 PROCESS = 4;
 constexpr u32 MEMORY = 32768 + (PROCESS * (65536 + 32768)); // global + (process (logic + local)) = 416 KiB
 constexpr u16 LOG = 1024;
}

namespace VIDEO {
 constexpr u16 WIDTH = 120;
 constexpr u16 HEIGHT = 160;
 constexpr u16 SPRITESHEET = 128;
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

namespace BUTTON {
 constexpr u8 COUNT = 8;
 constexpr u8 COUNT_KEY = 4;

 constexpr str KEY[COUNT][COUNT_KEY] = {
  {"up", "w", nullptr, nullptr},
  {"down", "s", nullptr, nullptr},
  {"left", "a", nullptr, nullptr},
  {"right", "d", nullptr, nullptr},
  {"space", "return", "j", "x"},
  {"left ctrl", "right ctrl", "k", "z"},
  {"left shift", "q", "l", nullptr},
  {"right shift", "e", "i", nullptr},
 };

 constexpr str PAD[COUNT] = {
  "dpup",
  "dpdown",
  "dpleft",
  "dpright",
  "a",
  "b",
  "leftshoulder",
  "rightshoulder",
 };
}

constexpr fpu SENTINEL = fpu::raw(0x80000000);
constexpr address_logic FARLAND = 0xFFFF;