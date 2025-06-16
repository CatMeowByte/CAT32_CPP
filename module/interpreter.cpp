#include "module/interpreter.hpp"
#include "module/opcode.hpp"

// TODO:
// handle warning of exception, overflow, etc
// decide wether newline is tokenized
// all inside /core should have namespace, as its a module for CAT32 kernel

namespace interpreter {
 vector<string> tokenize(const string& text) {
  vector<string> pack;
  string token;
  bool in_quote = false;

  for (u32 character = 0; character <= text.size(); character++) {
   char c = (character < text.size() ? text[character] : ' ');

   // handle quote (with backslash escape)
   if (c == '"') {
    u32 bs = 0;
    for (u32 pos = character - 1; pos >= 0 && text[pos] == '\\'; pos--) bs++;
    if (bs % 2 == 0) in_quote = !in_quote;
    token += c;
    continue;
   }

   // inside quote
   if (in_quote) {
    token += c;
    continue;
   }

   // boundary on space or end string
   if (c == ' ' || character == text.size()) {
    if (!token.empty()) {
     pack.push_back(token);
     token.clear();
    }
    continue;
   }

   // build token
   token += c;
  }

  return pack;
 }

 vector<u8> compile(const vector<string>& tokens) {
  vector<u8> bytecode;
  if (tokens.empty()) {return bytecode;}

  for (u32 i = 1; i < tokens.size(); i++) {
   bytecode.push_back(PUSH);
   bytecode.push_back(static_cast<u8>(stoi(tokens[i])));
  }

  bytecode.push_back(opcode_get(tokens[0].c_str()));
  bytecode.push_back(NOP);

  return bytecode;
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