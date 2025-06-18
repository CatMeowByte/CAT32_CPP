#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace interpreter {
 vector<string> tokenize(const string& text);
 vector<u32> compile(const vector<string>& tokens);
 void execute(const vector<u32>& bytecode);
}