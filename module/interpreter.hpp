#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace interpreter {
 vector<string> tokenize(const string& text);
 vector<u8> compile(const vector<string>& tokens);
 void execute(const vector<u8>& bytecode);
}