#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

enum class IndentType : u8 {
 UNKNOWN,
 IF,
 ELSE,
 WHILE,
 FUNCTION,
};

struct TokenLine {u8 indent; vector<string> tokens;};
struct Redirect {addr address = SENTINEL; vector<addr> pending;};
struct IndentFrame {addr jump_pos; addr header_start; IndentType type;};

namespace interpreter {
 TokenLine tokenize(const string& text);
 void compile(const TokenLine& tokens);
 void step();
}