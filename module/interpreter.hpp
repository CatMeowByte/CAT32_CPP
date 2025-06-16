#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace interpreter {
 vector<string> tokenize(const string& text);

 void execute(const vector<u8>& bytecode);
}