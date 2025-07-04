#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

struct TokenLine {u8 indent; vector<std::string> tokens;};

namespace interpreter {
 TokenLine tokenize(const string& text);
 void compile(const TokenLine& tokens);
 void step();
}