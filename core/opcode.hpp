#pragma once

#include "core/utility.hpp" // IWYU pragma: keep

#define OPCODES \
 /* stack */ \
 OP(0x11, push) \
 OP(0x12, pop) \
 /* memory */ \
 OP(0x13, takefrom) \
 OP(0x14, storeto) \
 /* OP(0x15, peek) */ \
 /* OP(0x16, poke) */ \
 /* counter */ \
 OP(0x2A, subgo) \
 OP(0x2B, subret) \
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
 /* builtin */ \
 OP(0xFF, call) \
  /* nop */ \
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

namespace opcode {
 u8 get(string cmd);
 bool exist(string cmd);
 string name(u8 value);
}

// boundary check
#define BAIL_IF_STACK_OVERFLOW if (!(stacker > 0)) {return SENTINEL;}
#define BAIL_UNLESS_STACK_ATLEAST(N) if (!(stacker <= SYSTEM::MEMORY - (N))) {return SENTINEL;}