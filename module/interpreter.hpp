#pragma once

#include "core/constant.hpp" // IWYU pragma: keep
#include "module/opcode.hpp" // IWYU pragma: keep

enum class IndentType : u8 {
 UNKNOWN = 0x00,
 IF = 0x1A,
 ELSE = 0x1F,
 WHILE = 0x2A,
};

struct TokenLine {u8 indent; vector<std::string> tokens;};
struct Redirect {u32 address = SENTINEL; vector<u32> pending;};
struct IndentFrame {u32 jump_pos; u32 loop_start; IndentType type;};

namespace interpreter {
 TokenLine tokenize(const string& text);
 void compile(const TokenLine& tokens);
 void step();
}