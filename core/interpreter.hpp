#pragma once

#include "spec/spec.hpp" // IWYU pragma: keep

namespace interpreter {
 enum TokenType {
  NIL,
  CMD,
  INT,
  STR,
 };

 struct Token {
 TokenType type;
  string value;
 };

 using Line = vector<Token>;

 Line tokenize(const string& text);

 void execute(const Line& line);
}