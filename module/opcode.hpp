#pragma once

#include "core/utility.hpp" // IWYU pragma: keep

#define OPCODES \
 OP(0x11, push) \
 OP(0x12, pop) \
 OP(0x13, pushm) \
 OP(0x14, popm) \
 \
 OP(0x2D, jump) \
 OP(0x2E, jumz) \
 OP(0x2F, junz) \
 \
 OP(0x31, add) \
 OP(0x32, sub) \
 OP(0x33, mul) \
 OP(0x34, div) \
 \
 OP(0x41, eq) \
 OP(0x42, neq) \
 OP(0x43, gt) \
 OP(0x44, lt) \
 OP(0x45, geq) \
 OP(0x46, leq) \
 \
 OP(0xA0, clear) \
 OP(0xA1, pixel) \
 OP(0xAF, flip) \
 OP(0x00, nop)

constexpr u32 SENTINEL = 0xFFFFFFFF;

namespace op {
 #define OP(hex, name) constexpr u8 name = hex;
 OPCODES
 #undef OP
};

namespace opfunc {
 #define OP(hex, name) u32 name(u32 value);
 OPCODES
 #undef OP
}

constexpr u8 opcode_get(const char *cmd) {
 switch (utility::hash(cmd)) {
  #define OP(hex, name) case utility::hash(#name): return op::name;
  OPCODES
  #undef OP
 default: return op::nop;
 }
}

constexpr str opcode_name(u8 value) {
 return
  #define OP(hex, name) value == hex ? #name :
  OPCODES
  #undef OP
  "UNKNOWN";
}