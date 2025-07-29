#pragma once

#include "core/utility.hpp" // IWYU pragma: keep

#define OPCODES \
 /* stack */ \
 OP(0x11, push) \
 OP(0x12, pop) \
 /* memory */ \
 OP(0x13, takefrom) \
 OP(0x14, storeto) \
 OP(0x15, peek) \
 OP(0x16, poke) \
 /* counter */ \
 OP(0x2D, jump) \
 OP(0x2E, jumz) \
 OP(0x2F, junz) \
 /* math */ \
 OP(0x31, add) \
 OP(0x32, sub) \
 OP(0x33, mul) \
 OP(0x34, div) \
 OP(0x35, neg) \
 /* logic */ \
 OP(0x41, eq) \
 OP(0x42, neq) \
 OP(0x43, gt) \
 OP(0x44, lt) \
 OP(0x45, geq) \
 OP(0x46, leq) \
 OP(0x47, land) \
 OP(0x48, lor) \
 OP(0x49, lnot) \
 /* bit */ \
 OP(0x4A, band) \
 OP(0x4B, bor) \
 OP(0x4C, bnot) \
 OP(0x4D, bshl) \
 OP(0x4E, bshr) \
 /* video */ \
 OP(0xA0, clear) \
 OP(0xA1, pixel) \
 OP(0xA2, text) \
 OP(0xAF, flip) \
 /* misc */ \
 OP(0xFA, see) \
 OP(0xFF, wait) \
 OP(0x00, nop)

namespace op {
 #define OP(hex, name) constexpr u8 name = hex;
 OPCODES
 #undef OP
};

namespace opfunc {
 #define OP(hex, name) addr name(elem value);
 OPCODES
 #undef OP
}

constexpr u8 opcode_get(str cmd) {
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