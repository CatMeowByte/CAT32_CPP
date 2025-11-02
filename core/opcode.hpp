#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

#define OPCODES \
 /* stack */ \
 OPI(0x11, push) \
 OPI(0x12, pop) \
 /* memory */ \
 OPI(0x13, takefrom) \
 OPI(0x14, storeto) \
 OPC(0x15, peek, 1) \
 OPC(0x16, poke, 2) \
 /* counter */ \
 OPI(0x2A, subgo) \
 OPI(0x2B, subret) \
 OPI(0x2D, jump) \
 OPI(0x2E, jumz) \
 OPC(0x2F, junz, 1) \
 /* math */ \
 OPC(0x31, add, 2) \
 OPC(0x32, sub, 2) \
 OPC(0x33, mul, 2) \
 OPC(0x34, div, 2) \
 OPC(0x35, neg, 1) \
 /* logic */ \
 OPC(0x41, eq, 2) \
 OPC(0x42, neq, 2) \
 OPC(0x43, gt, 2) \
 OPC(0x44, lt, 2) \
 OPC(0x45, geq, 2) \
 OPC(0x46, leq, 2) \
 OPC(0x47, land, 2) \
 OPC(0x48, lor, 2) \
 OPC(0x49, lnot, 1) \
 /* bit */ \
 OPC(0x4A, band, 2) \
 OPC(0x4B, bor, 2) \
 OPC(0x4C, bnot, 1) \
 OPC(0x4D, bshl, 2) \
 OPC(0x4E, bshr, 2) \
 /* module */ \
 OPI(0xFF, call) \
 /* nop */ \
 OPI(0x00, nop)

namespace op {
 #define OPI(hex, name) constexpr octo name = hex;
 #define OPC(hex, name, args) constexpr octo name = hex;
 OPCODES
 #undef OPI
 #undef OPC
}

namespace opfunc {
 #define OPI(hex, name) addr name(fpu value);
 #define OPC(hex, name, args) addr name(fpu value);
 OPCODES
 #undef OPI
 #undef OPC
}

namespace opcode {
 u8 get(string cmd);
 u8 args_count(string cmd);
 bool exist(string cmd);
 string name(u8 value);
}

// return
#define OPDONE return SENTINEL

// boundary check
#define BAIL_IF_STACK_OVERFLOW {using namespace memory::vm::process::app::ram_local; if (cast(addr, stacker) <= 0) {OPDONE;}}
#define BAIL_UNLESS_STACK_ATLEAST(N) {using namespace memory::vm::process::app::ram_local; if (cast(addr, stacker) >= field_address + field_length - (N)) {OPDONE;}}