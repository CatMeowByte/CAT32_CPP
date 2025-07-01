#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace interpreter {
 vector<string> tokenize(const string& text);
 void compile(const vector<string>& tokens);
 void step();
}