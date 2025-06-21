#include "module/interpreter.hpp"
#include "core/utility.hpp"
#include "module/opcode.hpp"

// TODO:
// handle warning of exception, overflow, etc

unordered_map<string, u32> symbols;

u32 slotter = 0;

static const unordered_map<string, u32> math_operations = {
 {"+", op::add},
 {"-", op::sub},
 {"*", op::mul},
 {"/", op::div},
 {"==", op::eq},
 {"!=", op::neq},
 {"<", op::lt},
 {">", op::gt},
 {"<=", op::leq},
 {">=", op::geq},
};

static void bytecode_append(vector<u32>& bytecode, u32 opcode, u32 operand);
static vector<string> breakdown(const string& expression);
static vector<string> shunting_yard(const vector<string>& tokens);

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

  for (const string& t : tokens) {cout << t << " ";}
  cout << endl;

  // arguments
  for (u32 i = 0; i < tokens.size(); i++) {
   // math
   bool has_math_operation = false;
   for (const auto& [key, val] : math_operations) {if (tokens[i].find(key) != string::npos) {has_math_operation = true; break;}}
   if (has_math_operation) {
    vector<string> postfix = shunting_yard(breakdown(tokens[i]));

    for (string t : postfix) {
     if (utility::is_number(t.c_str())) {
      bytecode_append(bytecode, op::push, stoi(t));
     }
     else if (symbols.count(t)) {
      bytecode_append(bytecode, op::pushm, symbols[t]);
     }
     else if (math_operations.count(t)) {
      bytecode_append(bytecode, math_operations.at(t), op::nop);
     }
    }
    continue;
   }

   // variable
   if (symbols.count(tokens[i]) && i != 0) {
    bytecode_append(bytecode, op::pushm, symbols[tokens[i]]);
    continue;
   }

   // number
   if (utility::is_number(tokens[i].c_str())) {
    bytecode_append(bytecode, op::push, stoi(tokens[i]));
    continue;
   }
  }

  // variable declaration and reassignment
  bool is_declare = (tokens[0] == "VAR" && tokens[2] == "=" && tokens.size() == 4);
  bool is_assign = (symbols.count(tokens[0]) && tokens[1] == "=" && tokens.size() == 3);

  if (is_declare || is_assign) {
   string name = is_declare ? tokens[1] : tokens[0];
   u32 address;

   if (is_declare) {
    address = slotter++;
    symbols[name] = address;
   } else {
    address = symbols[name];
   }

   bytecode_append(bytecode, op::popm, address);

   return bytecode;
  }

  // command
  string cmd_l = tokens[0];
  transform(cmd_l.begin(), cmd_l.end(), cmd_l.begin(), ::tolower);
  u32 cmd = opcode_get(cmd_l.c_str());
  if (cmd != op::nop) {
   bytecode_append(bytecode, cmd, op::nop);
  }

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

void bytecode_append(vector<u32>& bytecode, u32 opcode, u32 operand) {
 bytecode.push_back(opcode);
 bytecode.push_back(operand);

 // debug output
 string name = opcode_name(opcode);
 bool has_operand = (name.rfind("pop", 0) == 0 || name.rfind("push", 0) == 0);
 string value = has_operand ? to_string(operand) : "";
 cout << name << "\t" << value << endl;
}

static vector<string> breakdown(const string& expression) {
 vector<string> tokens;
 string token;

 for (u32 i = 0; i < expression.size(); i++) {
  char c = expression[i];

  // multi-char operators
  if (i + 1 < expression.size()) {
   string two_chars = expression.substr(i, 2);
   if (two_chars == "==" || two_chars == "!=" || two_chars == "<=" || two_chars == ">=") {
    if (!token.empty()) {
     tokens.push_back(token);
     token.clear();
    }
    tokens.push_back(two_chars);
    i++; // skip next char
    continue;
   }
  }

  // skip whitespace
  if (c == ' ') {continue;}

  // number/decimal (digits or one dot)
  if (isdigit(c) || (c == '.' && !token.empty() && all_of(token.begin(), token.end(), [](char tc){return isdigit(tc);}) && token.find('.') == string::npos)) {
   token += c;
   continue;
  }

  // variable (alphabet or _)
  if (isalpha(c) || c == '_') {
   token += c;
   continue;
  }

  // negative number: unary minus at start or after ( or operator
  if (c == '-' && (i == 0 || expression[i-1] == '(' || strchr("+-*/", expression[i-1]))) {
   token += c;
   continue;
  }

  // operator or parenthesis
  if (strchr("+-*/()<>", c)) {
   if (!token.empty()) {
    tokens.push_back(token);
    token.clear();
   }
   tokens.push_back(string(1, c));
   continue;
  }
 }

 if (!token.empty()) {tokens.push_back(token);}

 return tokens;
}

static u32 precedence(const string& op) {
 if (op == "+" || op == "-") {return 2;}
 if (op == "*" || op == "/") {return 3;}
 if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {return 1;}
 return 0;
}

static bool is_left_assoc(const string& op) {
 return true; // all basic ops are left-associative
}

static bool is_operator(const string& t) {
 return
  t == "+"  || t == "-"  ||
  t == "*"  || t == "/"  ||
  t == "==" || t == "!=" ||
  t == "<"  || t == ">"  ||
  t == "<=" || t == ">=";
}

static vector<string> shunting_yard(const vector<string>& tokens) {
 vector<string> output;
 vector<string> operator_stack;

 for (u32 i = 0; i < tokens.size(); i++) {
  const string& t = tokens[i];
  if (is_operator(t)) {
   while (
    !operator_stack.empty()
    && is_operator(operator_stack.back())
    && (
     (precedence(operator_stack.back()) > precedence(t))
     || (
      precedence(operator_stack.back()) == precedence(t)
      && is_left_assoc(t)
     )
    )
   ) {
    output.push_back(operator_stack.back());
    operator_stack.pop_back();
   }
   operator_stack.push_back(t);
  }
  else if (t == "(") {
   operator_stack.push_back(t);
  }
  else if (t == ")") {
   while (!operator_stack.empty() && operator_stack.back() != "(") {
    output.push_back(operator_stack.back());
    operator_stack.pop_back();
   }
   if (!operator_stack.empty() && operator_stack.back() == "(")
    operator_stack.pop_back();
  }
  else {
   // number or variable
   output.push_back(t);
  }
 }

 // pop any remaining ops
 while (!operator_stack.empty()) {
  output.push_back(operator_stack.back());
  operator_stack.pop_back();
 }

 return output;
}