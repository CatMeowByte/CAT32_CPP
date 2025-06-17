#pragma once

#include "core/utility.hpp" // IWYU pragma: keep

#define OPCODES \
 OP(0x11, PUSH, push) \
 OP(0x12, POP, pop) \
 OP(0x21, PUSHM, pushm) \
 OP(0x22, POPM, popm) \
 OP(0xA0, CLEAR, clear) \
 OP(0xA1, PIXEL, pixel) \
 OP(0xAF, FLIP, flip) \
 OP(0x00, NOP, nop)

enum Opcode : u8 {
 #define OP(hex, name, func) name = hex,
 OPCODES
 #undef OP
};

constexpr u8 opcode_get(const char *cmd) {
 switch (utility::hash(cmd)) {
  #define OP(hex, name, func) case utility::hash(#name): return name;
  OPCODES
  #undef OP
 default: return NOP;
 }
}

constexpr str opcode_name(u8 value) {
 return
  #define OP(hex, name, func) value == hex ? #name :
  OPCODES
  #undef OP
  "UNKNOWN";
}

namespace op {
 #define OP(hex, name, func) u8 func(u8 value);
 OPCODES
 #undef OP
}