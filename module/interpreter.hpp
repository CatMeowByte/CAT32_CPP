#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

enum class IndentType : u8 {
 UNKNOWN = 0x00,
 IF = 0x1A,
 ELSE = 0x1F,
 WHILE = 0x2A,
};

struct TokenLine {u8 indent; vector<std::string> tokens;};
struct Redirect {addr address = SENTINEL; vector<addr> pending;};
struct IndentFrame {addr jump_pos; addr loop_start; IndentType type;};

namespace interpreter {
 TokenLine tokenize(const string& text);
 void compile(const TokenLine& tokens);
 void step();
}