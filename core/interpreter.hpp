#pragma once

#include "spec/spec.hpp"

enum TokenType {
 NIL,
 CMD,
 INT,
 STR
};

struct Token {
 TokenType type;
 string value;
};

vector<Token> tokenize(const string& line);