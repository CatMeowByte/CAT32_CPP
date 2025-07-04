#include "module/interpreter.hpp"
#include "core/memory.hpp"
#include "core/utility.hpp"
#include "module/opcode.hpp"

// TODO:
// handle warning of exception, overflow, etc

hash_map<string, u32> symbols;

hash_map<string, Redirect> redirect;

vector<IndentFrame> indent_stack;
u8 indent_previous = 0;
IndentType indent_type_pending = IndentType::UNKNOWN;
u32 loop_start = 0;

static const hash_map<string, u32> math_operations = {
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
 {"&", op::land},
 {"|", op::lor},
 {"!", op::lnot},
 {"&&", op::band},
 {"||", op::bor},
 {"~", op::bnot},
 {"<<", op::bshl},
 {">>", op::bshr},
};

static void bytecode_append(u32 opcode, u32 operand);
static vector<string> breakdown(const string& expression);
static vector<string> shunting_yard(const vector<string>& tokens);

namespace interpreter {
 TokenLine tokenize(const string& line) {
  u8 indent = 0;
  while (indent < line.size() && line[indent] == ' ') {
   indent++;
  }
  const string text = line.substr(indent);

  vector<string> pack;
  string token;
  bool in_quote = false;

  for (u32 character = 0; character <= text.size(); character++) {
   const char c = (character < text.size() ? text[character] : ' ');

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

  return {indent, pack};
 }

 void compile(const TokenLine& line) {
  const u8& indent = line.indent;
  const vector<string>& tokens = line.tokens;

  if (tokens.empty()) {return;}

  for (const string& t : tokens) {cout << t << " ";}
  cout << endl;

  bool is_block_if = (tokens[0] == "IF" && tokens.back().back() == ':');
  bool is_block_else = (tokens[0] == "ELSE:");
  bool is_block_while = (tokens[0] == "WHILE" && tokens.back().back() == ':');

  if (is_block_if || is_block_while) {
   loop_start = writer;
  }

  // indent
  if (indent > indent_previous) {
   cout << "INDENT" << endl;

   const u32 last_opcode = bytecode[writer - 2];
   if (last_opcode != op::jump && last_opcode != op::jumz && last_opcode != op::junz) {
    cout << "WARNING: last opcode before indent is not jump/jumz/junz" << endl;
   }

   const u32 jump_pos = writer - 1;
   indent_stack.push_back({jump_pos, loop_start, indent_type_pending});
   indent_type_pending = IndentType::UNKNOWN;
   cout << "indent stack added with jump operand at " << jump_pos << endl;
  }

  // dedent
  if (indent < indent_previous) {
   cout << "DEDENT" << endl;

   if (is_block_else) {bytecode_append(op::jump, SENTINEL);}


   const IndentFrame& frame = indent_stack.back();
   const u32& jump_pos = frame.jump_pos;

   if (frame.type == IndentType::WHILE) {
    bytecode_append(op::jump, frame.loop_start); // next opcode
   }

   bytecode[jump_pos] = writer;
   cout << "patching jump operand at " << jump_pos << " to address " << writer << endl;

   indent_stack.pop_back();

   if (is_block_else) {
    indent_stack.push_back({writer - 1, loop_start, IndentType::ELSE});
    cout << "else jump at operand " << writer - 1 << endl;
   }
  }
  indent_previous = indent;

  // label
  if (tokens[0].size() > 1 && tokens[0].back() == ':') {
   string label = tokens[0].substr(0, tokens[0].size() - 1);
   redirect[label].address = writer;
   cout << label << " is at address " << writer << endl;

   for (u32 pos : redirect[label].pending) {bytecode[pos] = writer;}

   if (!redirect[label].pending.empty()) {
    cout << "pending [";
    for (u32 addr : redirect[label].pending) {cout << addr << " ";}
    cout << "] set to address " << writer << endl;
   }

   redirect[label].pending.clear();
   return;
  }

  // goto
  if (tokens[0] == "GOTO" && tokens.size() == 2) {
   string label = tokens[1];
   u32 target = redirect[label].address;

   bytecode_append(op::jump, target);
   if (target == SENTINEL) {
    u32 pending_addr = writer - 1;
    redirect[label].pending.push_back(pending_addr);
    cout << "address " << label << " unknown; [" << pending_addr << "] added to pending" << endl;
   } else {
    cout << "address " << label << " is " << target << endl;
   }

   return;
  }

  // argument
  bool is_declare = (tokens[0] == "VAR" && tokens[2] == "=" && tokens.size() == 4);
  bool is_assign = (symbols.count(tokens[0]) && tokens[1] == "=" && tokens.size() == 3);

  // FIXME:
  // not for arrray
  u32 argument_skip = 0;
  if (is_declare) {argument_skip = 3;}
  else if (is_assign) {argument_skip = 2;}

  // arguments
  for (u32 i = argument_skip; i < tokens.size(); i++) {
   vector<string> postfix = shunting_yard(breakdown(tokens[i]));

   for (const string& t : postfix) {
    if (utility::is_number(t.c_str())) {
     bytecode_append(op::push, stoi(t));
    }
    else if (symbols.count(t)) {
     bytecode_append(op::pushm, symbols[t]);
    }
    else if (math_operations.count(t)) {
     bytecode_append(math_operations.at(t), op::nop);
    }
   }
  }

  // if
  if (is_block_if) {
   bytecode_append(op::jumz, SENTINEL);
   indent_type_pending = IndentType::IF;
  }

  // while
  if (is_block_while) {
   bytecode_append(op::jumz, SENTINEL);
   indent_type_pending = IndentType::WHILE;
  }

  // variable declaration and reassignment
  if (is_declare || is_assign) {
   string name = is_declare ? tokens[1] : tokens[0];
   u32 address;

   if (is_declare) {
    address = slotter++;
    symbols[name] = address;
   } else {
    address = symbols[name];
   }

   bytecode_append(op::popm, address);

   if (is_declare) {cout << name << " is stored in " << address << endl;}

   return;
  }

  // command
  string cmd_l = tokens[0];
  transform(cmd_l.begin(), cmd_l.end(), cmd_l.begin(), ::tolower);
  u32 cmd = opcode_get(cmd_l.c_str());
  if (cmd != op::nop) {
   bytecode_append(cmd, op::nop);
  }

  return;
 }

 void step() {
  if (counter >= writer) {return;}

  u32 opcode = bytecode[counter];
  u32 operand = bytecode[counter + 1];
  u32 result = SENTINEL;

  switch (opcode) {
   #define OP(hex, name) case op::name: result = opfunc::name(operand); break;
   OPCODES
   #undef OP
  }

  if (result == SENTINEL) {counter += 2;}
  else {counter = result;}
 }
}

void bytecode_append(u32 opcode, u32 operand) {
 string name = opcode_name(opcode);
 if (name.length() < 4) {name += string(4 - name.length(), ' ');}

 bool has_operand = (name.rfind("pop", 0) == 0 || name.rfind("push", 0) == 0 || name.rfind("ju", 0) == 0);
 string value = has_operand ? (operand == SENTINEL ? "SENTINEL" : to_string(operand)) : "";

 cout << "[" << writer << "] " << name << "\t[" << writer + 1 << "] " << value << endl;

 bytecode[writer++] = opcode;
 bytecode[writer++] = operand;
}

static vector<string> breakdown(const string& expression) {
 vector<string> tokens;
 string token;

 for (u32 i = 0; i < expression.size(); i++) {
  char c = expression[i];

  // multi-char operators
  if (i + 1 < expression.size()) {
   string two_chars = expression.substr(i, 2);
   if (
    two_chars == "==" || two_chars == "!=" || two_chars == "<=" || two_chars == ">=" ||
    two_chars == "&&" || two_chars == "||" || two_chars == "<<" || two_chars == ">>"
   ) {
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
  if (strchr("+-*/()<>!&|~", c)) {
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
 if (op == "!" || op == "~") {return 5;}
 if (op == "*" || op == "/") {return 4;}
 if (op == "+" || op == "-") {return 3;}
 if (op == "<<" || op == ">>") {return 2;}
 if (op == "<" || op == ">" || op == "<=" || op == ">=") {return 1;}
 if (op == "==" || op == "!=") {return 1;}
 if (op == "&&" || op == "||") {return 0;}
 if (op == "&" || op == "|") {return 0;}
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
  t == "<=" || t == ">=" ||
  t == "&"  || t == "|"  || t == "!" ||
  t == "&&" || t == "||" || t == "~" ||
  t == "<<" || t == ">>";
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