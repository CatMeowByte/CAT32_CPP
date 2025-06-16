#include "module/interpreter.hpp"
#include "module/opcode.hpp"

// TODO:
// handle warning of exception, overflow, etc
// decide wether newline is tokenized
// all inside /core should have namespace, as its a module for CAT32 kernel

namespace interpreter {
 vector<string> tokenize(const string& text) {
  vector<string> tokens;
  // string token;
  // int index = 0;
  // bool in_quote = false;

  // for (u32 character = 0; character <= text.size(); character++) {
  //  char c = (character < text.size() ? text[character] : ' ');

  //  // quote
  //  // also check if no backslash behind it
  //  if (c == '"') {
  //   int bs = 0;
  //   for (int pos = character - 1; pos >= 0 && text[pos] == '\\'; pos--) bs++;
  //   if (bs % 2 == 0) in_quote = !in_quote;
  //   token += c;
  //   continue;
  //  }

  //  // token inside quote
  //  if (in_quote) {
  //   token += c;
  //   continue;
  //  }

  //  // token boundary (space)
  //  if (c == ' ' || character == text.size()) {
  //   if (!token.empty()) {
  //    // categorize token
  //    TokenType type = NIL;

  //    // CMD
  //    if (index == 0) {type = CMD;}

  //    // INT
  //    else if (token.find_first_not_of("0123456789-+") == string::npos) {type = INT;}

  //    // STR
  //    else if (token.front() == '"' && token.back() == '"') {
  //     type = STR;
  //     token = token.substr(1, token.size() - 2); // remove quotes
  //     string unescaped;
  //     for (u32 i = 0; i < token.size(); i++) {
  //      if (token[i] == '\\' && i + 1 < token.size()) {
  //       i++;
  //       if (token[i] == 'n') {unescaped += '\n';}
  //       else if (token[i] == 't') {unescaped += '\t';}
  //       else if (token[i] == '\\') {unescaped += '\\';}
  //       else if (token[i] == '"') {unescaped += '"';}
  //       else {unescaped += token[i];}
  //      } else {
  //       unescaped += token[i];
  //      }
  //     }
  //     token = unescaped;
  //    }

  //    tokens.push_back({type, token});
  //    token.clear();
  //    index++;
  //   }
  //   continue;
  //  }
  //  // default: add char to token
  //  token += c;
  // }
  // // blank
  // if (tokens.empty()) {tokens.push_back({NIL, ""});}

  return tokens;
 }

 void execute(const vector<u8>& bytecode) {
  for (u32 counter = 0; counter < bytecode.size(); counter += 2) {
   u8 opcode = bytecode[counter];
   u8 operand = bytecode[counter + 1];

   switch (opcode) {
    case PUSH:
     op::push(operand);
     break;
    case CLEAR:
     op::clear();
     break;
    case PIXEL:
     op::pixel();
     break;
    case FLIP:
     op::flip();
     break;
    case NOP:
     break;
   }
  }
 }
}