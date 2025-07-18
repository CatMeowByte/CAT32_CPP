#include "core/define.hpp"
#include "core/memory.hpp"
#include "core/utility.hpp"
#include "module/interpreter.hpp"
#include "module/opcode.hpp"

// TODO:
// handle warning of exception, overflow, etc

hash_map<string, SymbolData> symbols;

hash_map<string, Redirect> redirect;

vector<IndentFrame> indent_stack;
u8 indent_previous = 0;
IndentType indent_type_pending = IndentType::UNKNOWN;
addr loop_start = 0;

enum Precedence : u8 {
 BASE = 0,
 LOGIC = 0,
 COMPARE = 1,
 SHIFT = 2,
 ADD = 3,
 MUL = 4,
 UNARY = 5,
};

#define SORTED_OPERATORS \
 OP("==", op::eq, COMPARE) \
 OP("!=", op::neq, COMPARE) \
 OP("<=", op::leq, COMPARE) \
 OP(">=", op::geq, COMPARE) \
 OP("&&", op::band, LOGIC) \
 OP("||", op::bor, LOGIC) \
 OP("~~", op::bnot, UNARY) \
 OP("<<", op::bshl, SHIFT) \
 OP(">>", op::bshr, SHIFT) \
 OP("+", op::add, ADD) \
 OP("-", op::sub, ADD) \
 OP("*", op::mul, MUL) \
 OP("/", op::div, MUL) \
 OP("<", op::lt, COMPARE) \
 OP(">", op::gt, COMPARE) \
 OP("&", op::land, LOGIC) \
 OP("|", op::lor, LOGIC) \
 OP("!", op::lnot, UNARY)

static const hash_map<string, u8> math_opcodes = {
#define OP(sym, code, prec) {sym, code},
 SORTED_OPERATORS
#undef OP
{"NEG", op::neg},
};

static const set<string> math_list_operations = {
#define OP(sym, code, prec) sym,
 SORTED_OPERATORS
#undef OP
};

static const set<string> math_list_operations_bracket = {
#define OP(sym, code, prec) sym,
 SORTED_OPERATORS
#undef OP
 "(",
 ")",
};

static void bytecode_append(u8 opcode, elem operand);
static vector<string> breakdown(const string& expression);
static vector<string> postfix(const vector<string>& tokens);

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

  // flags
  enum BlockType : u8 {BLOCK_NONE, BLOCK_IF, BLOCK_WHILE, BLOCK_ELSE};
  BlockType block_type = BLOCK_NONE;

  enum AssignType : u8 {ASSIGN_NONE, ASSIGN_DECLARE, ASSIGN_SET};
  AssignType assign_type = ASSIGN_NONE;

  enum DeclareStyle : u8 {DECLARE_NONE, DECLARE_VAR, DECLARE_STRIPE_SIZE, DECLARE_STRIPE_FULL};
  DeclareStyle declare_style = DECLARE_NONE;

  bool is_expression = (find(tokens.begin(), tokens.end(), "=") == tokens.end());

  for (const string& t : tokens) {cout << t << " ";}
  cout << endl;

  if (tokens[0] == "IF" && tokens.back().back() == ':') {block_type = BLOCK_IF;}
  if (tokens[0] == "WHILE" && tokens.back().back() == ':') {block_type = BLOCK_WHILE;}
  if (tokens[0] == "ELSE:") {block_type = BLOCK_ELSE;}

  if (block_type == BLOCK_IF || block_type == BLOCK_WHILE) {
   loop_start = writer;
  }

  // indent
  if (indent > indent_previous) {
   cout << "INDENT" << endl;

   const elem last_opcode = bytecode[writer - 2];
   if (last_opcode != op::jump && last_opcode != op::jumz && last_opcode != op::junz) {
    cout << "WARNING: last opcode before indent is not jump/jumz/junz" << endl;
   }

   const addr jump_pos = writer - 1;
   indent_stack.push_back({jump_pos, loop_start, indent_type_pending});
   indent_type_pending = IndentType::UNKNOWN;
   cout << "indent stack added with jump operand at " << jump_pos << endl;
  }

  // dedent
  if (indent < indent_previous) {
   cout << "DEDENT" << endl;

   if (block_type == BLOCK_ELSE) {bytecode_append(op::jump, SENTINEL);}

   const IndentFrame& frame = indent_stack.back();
   const addr& jump_pos = frame.jump_pos;

   if (frame.type == IndentType::WHILE) {
    bytecode_append(op::jump, frame.loop_start); // next opcode
   }

   bytecode[jump_pos] = writer;
   cout << "patching jump operand at " << jump_pos << " to address " << writer << endl;

   indent_stack.pop_back();

   if (block_type == BLOCK_ELSE) {
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

   for (addr pos : redirect[label].pending) {bytecode[pos] = writer;}

   if (!redirect[label].pending.empty()) {
    cout << "pending [";
    for (addr address : redirect[label].pending) {cout << address << " ";}
    cout << "] set to address " << writer << endl;
   }

   redirect[label].pending.clear();
   return;
  }

  // goto
  if (tokens[0] == "GOTO" && tokens.size() == 2) {
   string label = tokens[1];
   addr target = redirect[label].address;

   bytecode_append(op::jump, target);
   if (target == cast(u32, SENTINEL)) {
    addr pending = writer - 1;
    redirect[label].pending.push_back(pending);
    cout << "address " << label << " unknown; [" << pending << "] added to pending" << endl;
   } else {
    cout << "address " << label << " is " << target << endl;
   }

   return;
  }

  // variable set check
  if (tokens[0] == "VAR" && tokens[2] == "=" && tokens.size() == 4) {assign_type = ASSIGN_DECLARE; declare_style = DECLARE_VAR;}
  if (symbols.count(tokens[0]) && tokens[1] == "=" && tokens.size() == 3) {assign_type = ASSIGN_SET;}

  // arguments
  for (u32 i = 0; i < tokens.size(); i++) {
   if (tokens[i] == "=") {is_expression = true; continue;}

   vector<string> expression_ordered = postfix(breakdown(tokens[i]));
   for (const string& t : expression_ordered) {
    if (utility::is_number(t.c_str())) {
     bytecode_append(op::push, cast(elem, round(fpu::scale(stod(t))))); // fixed point
    }
    else if (symbols.count(t)) {
     const SymbolData& symbol = symbols[t];
     switch (symbol.type) {
      case NUMBER: {
       if (!is_expression) {continue;}
       bytecode_append(op::takefrom, symbol.address);
       break;
      }
      default: {break;}
     }
    }
    else if (math_opcodes.count(t)) {
     bytecode_append(math_opcodes.at(t), op::nop);
    }
   }
  }

  // if
  if (block_type == BLOCK_IF) {
   bytecode_append(op::jumz, SENTINEL);
   indent_type_pending = IndentType::IF;
  }

  // while
  if (block_type == BLOCK_WHILE) {
   bytecode_append(op::jumz, SENTINEL);
   indent_type_pending = IndentType::WHILE;
  }

  // variable declaration and reassignment
  if (assign_type == ASSIGN_DECLARE) {
   string name = tokens[1];
   addr address = 0;

   switch (declare_style) {
    case DECLARE_VAR: {
     address = slotter++;
     symbols[name].type = NUMBER;
     symbols[name].address = address;
     bytecode_append(op::storeto, address);
     cout << name << " is stored in " << address << endl;
     break;
    }
    default: {break;}
   }
  } else if (assign_type == ASSIGN_SET) {
   string name = tokens[0];
   addr address = symbols[name].address;
   bytecode_append(op::storeto, address);
  }

  // command
  string command_lowercase = tokens[0];
  transform(command_lowercase.begin(), command_lowercase.end(), command_lowercase.begin(), ::tolower);
  u8 command = opcode_get(command_lowercase.c_str());
  if (command != op::nop) {
   bytecode_append(command, op::nop);
  }

  return;
 }

 void step() {
  if (counter >= writer) {return;}

  elem opcode = bytecode[counter];
  elem operand = bytecode[counter + 1];
  addr result = SENTINEL;

  switch (opcode) {
   #define OP(hex, name) case op::name: result = opfunc::name(operand); break;
   OPCODES
   #undef OP
  }

  if (result == cast(u32, SENTINEL)) {counter += 2;}
  else {counter = result;}
 }
}

void bytecode_append(u8 opcode, elem operand) {
 string name = opcode_name(opcode);
 if (name.length() < 4) {name += string(4 - name.length(), ' ');}

 bool has_operand = (
  name == "pop"    || name == "push" ||
  name == "takefrom" || name == "storeto" ||
  name == "jump"   || name == "jumz" || name == "junz"
 );
 string value = "";
 if (has_operand) {
  if (operand == SENTINEL) {value = "SENTINEL";}
  // else if (name == "pop" || name == "push") {value = utility::string_no_trailing(fpu::unscale(operand));}
  else {value = to_string(operand);}
 }

 cout << "[" << writer << "] " << name << "\t[" << writer + 1 << "] " << value << endl;

 bytecode[writer++] = opcode;
 bytecode[writer++] = operand;
}

static vector<string> breakdown(const string& expression) {
 vector<string> tokens;
 string token;
 bool is_negative = false;

 for (u32 i = 0; i < expression.size(); i++) {
  char c = expression[i];

  // negative number: unary minus at start or after ( or operator
  if (c == '-' && (i == 0 || expression[i-1] == '(' || math_list_operations.count(string(1, expression[i-1])))) {
   if (i+1 < expression.size() && (isdigit(expression[i+1]) || expression[i+1] == '.')) {
    token += c;
    continue;
   }
   is_negative = true;
   continue;
  }

  // operators
  bool operator_matched = false;
  for (u8 len = 3; len > 0 && i + len <= expression.size(); len--) {
   string candidate = expression.substr(i, len);
   if (math_list_operations_bracket.count(candidate)) {
    if (!token.empty()) {
     tokens.push_back(token);
     if (is_negative) {
      tokens.push_back("NEG");
      is_negative = false;
     }
     token.clear();
    }
    tokens.push_back(candidate);
    i += len - 1;
    operator_matched = true;
    break;
   }
  }
  if (operator_matched) {continue;}

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
 }

 if (!token.empty()) {
  tokens.push_back(token);
  if (is_negative) {
   tokens.push_back("NEG");
   is_negative = false;
  }
 }

 return tokens;
}

static u8 precedence(const string& op) {
#define OP(sym, code, prec) if (op == sym) {return prec;}
 SORTED_OPERATORS
#undef OP
 return BASE;
}

static vector<string> postfix(const vector<string>& tokens) {
 vector<string> output;
 vector<string> operator_stack;

 for (u32 i = 0; i < tokens.size(); i++) {
  const string& t = tokens[i];
  if (math_list_operations.count(t)) {
   while (
    !operator_stack.empty()
    && math_list_operations.count(operator_stack.back())
    && (
     (precedence(operator_stack.back()) > precedence(t))
     || (
      precedence(operator_stack.back()) == precedence(t)
      && true // is_left_assoc(t) // all basic ops are left-associative
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
   if (!operator_stack.empty() && operator_stack.back() == "(") {
    operator_stack.pop_back();
   }
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