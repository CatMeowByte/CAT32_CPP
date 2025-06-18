#include "module/interpreter.hpp"
#include "module/opcode.hpp"

// TODO:
// handle warning of exception, overflow, etc

unordered_map<string, u32> symbols;

u32 slotter = 0;

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

 vector<u32> compile(const vector<string>& tokens) {
  vector<u32> bytecode;
  if (tokens.empty()) {return bytecode;}

  if (tokens[0] == "VAR") {
   string name = tokens[1];
   u32 value = stoi(tokens[3]);
   u32 address = slotter++;
   symbols[name] = address;
   bytecode.push_back(op::push);
   bytecode.push_back(static_cast<u32>(value));
   bytecode.push_back(op::popm);
   bytecode.push_back(static_cast<u32>(address));
   return bytecode;
  }

  for (u32 i = 1; i < tokens.size(); i++) {
   if (symbols.count(tokens[i])) {
    string name = tokens[i];
    int address = symbols[name];
    bytecode.push_back(op::pushm);
    bytecode.push_back(static_cast<u32>(address));
   } else {
    bytecode.push_back(op::push);
    bytecode.push_back(static_cast<u32>(stoi(tokens[i])));
   }
  }

  bytecode.push_back(opcode_get(tokens[0].c_str()));
  bytecode.push_back(op::nop);

  return bytecode;
 }

 void execute(const vector<u32>& bytecode) {
  for (u32 counter = 0; counter < bytecode.size(); counter += 2) {
   u32 opcode = bytecode[counter];
   u32 operand = bytecode[counter + 1];

   switch (opcode) {
    #define OP(hex, name) case op::name: opfunc::name(operand); break;
    OPCODES
    #undef OP
   }
  }
 }
}