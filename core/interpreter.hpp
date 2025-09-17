#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

struct TokenLine {
 u8 indent; // = 0;
 vector<string> tokens;
};

struct Redirect {addr address = SENTINEL; vector<addr> pending;}; // TODO: delete goto jump likely incompatible with scope

namespace interpreter {
 TokenLine tokenize(const string& text);
 void compile(const TokenLine& tokens);
 void step();
}