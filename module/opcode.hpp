#pragma once

#include "core/utility.hpp" // IWYU pragma: keep

#define OPCODES \
 OP(0x11, PUSH) \
 OP(0x12, POP) \
 OP(0xA0, CLEAR) \
 OP(0xA1, PIXEL) \
 OP(0xAF, FLIP) \
 OP(0x00, NOP)

enum Opcode : u8 {
 #define OP(hex, name) name = hex,
  OPCODES
 #undef OP
};

constexpr u8 opcode_get(const char *cmd) {
 switch (utility::hash(cmd)) {
  #define OP(hex, name) case utility::hash(#name): return name;
   OPCODES
  #undef OP
 default: return NOP;
 }
}

constexpr str opcode_name(u8 value) {
 return
  #define OP(hex, name) value == hex ? #name :
   OPCODES
  #undef OP
  "UNKNOWN";
}

#undef OPCODES

namespace op {
 void push(u8 value);
 u8 pop();

 void clear();
 void pixel();
 void rect();
 void flip();
}