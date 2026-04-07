#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

#define OPCODES \
 /* stack */ \
 OPV(0x11, push) \
 /* memory */ \
 OPA(0x13, takefrom) \
 OPA(0x14, storeto) \
 OPA(0x16, stampto) \
 OP (0x1A, get) \
 OP (0x1B, set) \
 /* counter */ \
 OPA(0x2A, subgo) \
 OP (0x2B, subret) \
 OPA(0x2D, jump) \
 OPA(0x2E, jumz) \
 OPA(0x2F, junz) \
 /* math */ \
 OP (0x31, add) \
 OP (0x32, sub) \
 OP (0x33, mul) \
 OP (0x34, div) \
 OP (0x35, mod) \
 OP (0x36, neg) \
 /* logic */ \
 OP (0x41, eq) \
 OP (0x42, neq) \
 OP (0x43, gt) \
 OP (0x44, lt) \
 OP (0x45, geq) \
 OP (0x46, leq) \
 OP (0x47, land) \
 OP (0x48, lor) \
 OP (0x49, lnot) \
 /* bit */ \
 OP (0x4A, band) \
 OP (0x4B, bor) \
 OP (0x4C, bnot) \
 OP (0x4D, bshl) \
 OP (0x4E, bshr) \
 /* marker */ \
 OP (0xAA, prime) \
 /* module */ \
 OPV(0xFF, call) \
 /* nop */ \
 OP (0x00, nop)

namespace op {
 #define OP(hex, name) constexpr octo name = hex;
 #define OPA(hex, name) constexpr octo name = hex;
 #define OPV(hex, name) constexpr octo name = hex;
 OPCODES
 #undef OP
 #undef OPA
 #undef OPV
}

namespace op_call {
 #define OP(hex, name) address_logic name();
 #define OPA(hex, name) address_logic name();
 #define OPV(hex, name) address_logic name();
 OPCODES
 #undef OP
 #undef OPA
 #undef OPV
}

namespace opcode {
 u8 get(string cmd);
 bool exist(string cmd);
 string name(u8 value);
}

#define OPCODE(name, ...) \
 address_logic name() { \
 __VA_ARGS__ \
 return active::logic->counter.a() + 1;}

#define OPCODE_ADDRESS(name, ...) \
 address_logic name() { \
 address_logic operand = memory::unaligned_16_read(active::logic->code_octo + active::logic->counter.a() + 1); \
 __VA_ARGS__ \
 return active::logic->counter.a() + 3;}

#define OPCODE_VALUE(name, ...) \
 address_logic name() { \
 fpu operand = fpu::raw(memory::unaligned_32_read(active::logic->code_octo + active::logic->counter.a() + 1)); \
 __VA_ARGS__ \
 return active::logic->counter.a() + 5;}

// newline to supress macro warning "backslash-newline at end of file"