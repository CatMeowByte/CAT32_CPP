#include "core/interpreter.hpp"

// TODO: handle warning of exception, overflow, etc

vector<Token> tokenize(const string& line) {
 vector<Token> tokens;
 string token;
 int index = 0;
 bool in_quote = false;

 for (u16 character = 0; character <= line.size(); character++) {
  char c = (character < line.size() ? line[character] : ' ');

  // quote
  // also check if no backslash behind it
  if (c == '"') {
   int bs = 0;
   for (int pos = character - 1; pos >= 0 && line[pos] == '\\'; pos--) bs++;
   if (bs % 2 == 0) in_quote = !in_quote;
   token += c;
   continue;
  }

  // token inside quote
  if (in_quote) {
   token += c;
   continue;
  }

  // token boundary (space)
  if (c == ' ' || character == line.size()) {
   if (!token.empty()) {
    // categorize token
    TokenType type = NIL;

    // CMD
    if (index == 0) {type = CMD;}
    
    // INT
    else if (token.find_first_not_of("0123456789-+") == string::npos) {type = INT;}
    
    // STR
    else if (token.front() == '"' && token.back() == '"') {
     type = STR;
     token = token.substr(1, token.size() - 2); // remove quotes
     string unescaped;
     for (size_t i = 0; i < token.size(); i++) {
      if (token[i] == '\\' && i + 1 < token.size()) {
       i++;
       if (token[i] == 'n') {unescaped += '\n';}
       else if (token[i] == 't') {unescaped += '\t';}
       else if (token[i] == '\\') {unescaped += '\\';}
       else if (token[i] == '"') {unescaped += '"';}
       else {unescaped += token[i];}
      } else {
       unescaped += token[i];
      }
     }
     token = unescaped;
    }
    
    tokens.push_back({type, token});
    token.clear();
    index++;
   }
   continue;
  }

  // default: add char to token
  token += c;
 }
 return tokens;
}