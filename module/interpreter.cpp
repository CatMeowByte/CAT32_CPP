#include "core/constant.hpp"
#include "core/define.hpp"
#include "core/memory.hpp"
#include "core/utility.hpp"
#include "module/interpreter.hpp"
#include "module/opcode.hpp"

// TODO:
// handle warning of exception, overflow, etc

constexpr str SYMBOL_STRING_PREFIX = "&:";
constexpr str OPERATOR_OFFSET = "#";
constexpr u8 OPERATOR_COMMENT = '$';

hash_map<string, SymbolData> symbols;

hash_map<string, Redirect> redirect;

vector<IndentFrame> indent_stack;
u8 indent_previous = 0;
IndentType indent_type_pending = IndentType::UNKNOWN;
addr header_start = 0;

enum Precedence : u8 {
 BASE = 0,
 LOGIC = 0,
 COMPARE = 1,
 SHIFT = 2,
 ADD = 3,
 MUL = 4,
 OFFSET = 5,
 UNARY = 6,
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
};

static const hash_set<string> math_list_operations = {
#define OP(sym, code, prec) sym,
 SORTED_OPERATORS
#undef OP
 OPERATOR_OFFSET,
};

static const hash_set<string> math_list_operations_bracket = {
#define OP(sym, code, prec) sym,
 SORTED_OPERATORS
#undef OP
 OPERATOR_OFFSET,
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

   // quote (with backslash escape)
   if (c == '"') {
    u8 bs = 0;
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

   // comment (skip rest of line)
   if (!in_quote && c == OPERATOR_COMMENT) {break;}

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

  // flag

  enum class HeaderType : u8 {NONE, IF, WHILE, ELSE, FUNCTION};
  HeaderType header_type = HeaderType::NONE;

  enum class AssignType : u8 {NONE, DECLARE, SET};
  AssignType assign_type = AssignType::NONE;

  enum class DeclareStyle : u8 {NONE, VAR, STRING_FULL, STRING_SIZE, STRIPE_FULL, STRIPE_SIZE};
  DeclareStyle declare_style = DeclareStyle::NONE;

  enum class SetStyle : u8 {NONE, VAR, STRING, STRIPE};
  SetStyle set_style = SetStyle::NONE;

  bool is_expression = (find(tokens.begin(), tokens.end(), "=") == tokens.end());

  bool is_return = tokens[0] == "return";

  for (const string& t : tokens) {cout << t << " ";}
  cout << endl;

  if (tokens[0] == "if" && tokens.back().back() == ':') {header_type = HeaderType::IF;}
  if (tokens[0] == "while" && tokens.back().back() == ':') {header_type = HeaderType::WHILE;}
  if (tokens[0] == "else:") {header_type = HeaderType::ELSE;}
  if (tokens[0] == "func" && tokens.back().back() == ':') {header_type = HeaderType::FUNCTION;}

  if (header_type == HeaderType::IF || header_type == HeaderType::WHILE || header_type == HeaderType::FUNCTION) {
   header_start = writer;
  }

  // indent
  if (indent > indent_previous) {
   if (indent - indent_previous > 1) {cout << "WARNING: too much indent" << endl;}
   for (u8 indent_level = indent_previous + 1; indent_level <= indent; ++indent_level) {
    cout << "INDENT" << endl;

    if (indent_type_pending == IndentType::FUNCTION) {
     indent_stack.push_back({header_start + 1, header_start, indent_type_pending}); // reuse header_start for function
    }
    else {
     const elem last_opcode = bytecode[writer - 2];
     if (last_opcode != op::jump && last_opcode != op::jumz && last_opcode != op::junz) {
      cout << "WARNING: last opcode before indent is not jump/jumz/junz" << endl;
     }

     const addr jump_pos = writer - 1;
     indent_stack.push_back({jump_pos, header_start, indent_type_pending});
     indent_type_pending = IndentType::UNKNOWN;
     cout << "indent stack added with jump operand at " << jump_pos << endl;
    }
   }
  }

  // dedent
  if (indent < indent_previous) {
   for (u8 indent_level = indent_previous; indent_level > indent; --indent_level) {
    cout << "DEDENT" << endl;

    if (header_type == HeaderType::ELSE) {bytecode_append(op::jump, SENTINEL);}

    const IndentFrame& frame = indent_stack.back();
    const addr& jump_pos = frame.jump_pos;

    if (frame.type == IndentType::WHILE) {
     bytecode_append(op::jump, frame.header_start); // next opcode
    }

    bytecode[jump_pos] = writer;
    cout << "patching jump operand at " << jump_pos << " to address " << writer << endl;

    indent_stack.pop_back();

    if (header_type == HeaderType::ELSE) {
     indent_stack.push_back({writer - 1, header_start, IndentType::ELSE});
     cout << "else jump at operand " << writer - 1 << endl;
    }
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
  if (tokens[0] == "goto" && tokens.size() == 2) {
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

  // set type
  if (tokens[0] == "var" && tokens[2] == "=" && tokens.size() == 4) {
   assign_type = AssignType::DECLARE;
   declare_style = DeclareStyle::VAR;
  }
  if (tokens[0] == "string" && tokens[2] == "=" && tokens.size() == 4 && tokens[3].size() >= 2 && tokens[3].front() == '"' && tokens[3].back() == '"') {
   assign_type = AssignType::DECLARE;
   declare_style = DeclareStyle::STRING_FULL;
  }
  if (tokens[0] == "string" && tokens.size() == 3 && utility::is_number(tokens[2])) {
   assign_type = AssignType::DECLARE;
   declare_style = DeclareStyle::STRING_SIZE;
  }
  if (tokens[0] == "stripe" && tokens[2] == "=" && tokens.size() >= 4) {
   assign_type = AssignType::DECLARE;
   declare_style = DeclareStyle::STRIPE_FULL;
  }
  if (tokens[0] == "stripe" && tokens.size() == 3 && utility::is_number(tokens[2])) {
   assign_type = AssignType::DECLARE;
   declare_style = DeclareStyle::STRIPE_SIZE;
  }
  if (tokens.size() == 3 && tokens[1] == "=") {
   assign_type = AssignType::SET;
   set_style = SetStyle::VAR;

   u64 has_hash = tokens[0].find('#');
   if (has_hash != string::npos && symbols.count(tokens[0].substr(0, has_hash))) {set_style = SetStyle::STRIPE;}
   if (symbols.count(tokens[0]) && symbols[tokens[0]].type == STRING) {set_style = SetStyle::STRING;}
  }

  // token breakdown
  for (u32 i = 0; i < tokens.size(); i++) {
   const string& token = tokens[i];
   if (token == "=") {is_expression = true; continue;}

   vector<string> expression_breaked = breakdown(token);
   // cout << "BREAK: "; for (const std::string &token : expression_breaked) {cout << "'" << token << "' ";} cout << endl;
   vector<string> expression_ordered = postfix(expression_breaked);
   // cout << "SORT: "; for (const std::string &token : expression_ordered) {cout << "'" << token << "' ";} cout << endl;
   for (u32 j = 0; j < expression_ordered.size(); j++) {
    const string& token = expression_ordered[j];

    // command
    u8 command = opcode_get(token.c_str());
    if (command != op::nop) {
     bytecode_append(command, op::nop);
    }

    // string
    if (token.front() == '"' && token.back() == '"') {
     string content = token.substr(1, token.size() - 2); // strip quotes
     string name = SYMBOL_STRING_PREFIX + content;
     if (symbols.count(name)) {
      cout << "string already exist in " << symbols[name].address << " is written \"" << content << "\"" << endl;
      continue;
     }

     addr address = slotter;
     u32 length = content.size();
     addr slot_after = slotter + length + 1;
     if (set_style == SetStyle::STRING) {
      address = symbols[tokens[0]].address;
      slot_after = slotter;
     }
     bytecode_append(op::push, fpu::pack(length));
     for (u32 j = 0; j < length; j++) {
      bytecode_append(op::push, fpu::pack(content[j]));
     }

     for (s32 i = length; i >= 0; i--) {
      bytecode_append(op::storeto, address + i);
     }

     slotter = slot_after;

     if (declare_style == DeclareStyle::STRING_FULL || set_style == SetStyle::STRING) {continue;}
     // hidden declare
     symbols[name].type = STRING;
     symbols[name].address = slotter - (length + 1);

     cout << "string hidden declare in " << symbols[name].address << " is written \"" << content << "\"" << endl;

     // also push to stack for other use
     bytecode_append(op::push, fpu::pack(symbols[name].address));
    }

    // number
    // hardest part to document
    // all detail of rounding, casting, data type is important
    if ((utility::is_number(token) || utility::is_hex(token)) && declare_style != DeclareStyle::STRIPE_SIZE && declare_style != DeclareStyle::STRING_SIZE) {
     double value = utility::is_hex(token) ? utility::hex_to_number(token) : stod(token);
     bytecode_append(op::push, cast(elem, cast(s64, round(fpu::scale(value))))); // rounded fixed point
    }

    // symbol
    if (symbols.count(token)) {
     if (header_type == HeaderType::FUNCTION) {continue;}
     const SymbolData& symbol = symbols[token];
     switch (symbol.type) {
      case NUMBER: {
       if (assign_type == AssignType::SET && set_style == SetStyle::VAR && !is_expression && token == tokens[0]) {break;}
       bytecode_append(op::takefrom, symbol.address);
       break;
      }
      case STRING: {
       if (assign_type == AssignType::SET && set_style == SetStyle::STRING && !is_expression && token == tokens[0]) {break;}
       bytecode_append(op::push, fpu::pack(symbol.address));
       break;
      }
      case STRIPE: {
       bytecode_append(op::push, fpu::pack(symbol.address));
       break;
      }
      case FUNCTION: {
       bytecode_append(op::subgo, symbol.address);
       break;
      }
      default: {break;}
     }
    }

    // "#"
    if (token == OPERATOR_OFFSET) {
     bytecode_append(op::add, op::nop);
     if (assign_type == AssignType::SET && set_style == SetStyle::STRIPE && !is_expression && j == expression_ordered.size() - 1) {continue;}
     bytecode_append(op::peek, op::nop);
    }

    // math operations
    if (math_opcodes.count(token)) {
     bytecode_append(math_opcodes.at(token), op::nop);
    }

    // function declaration
    if (header_type == HeaderType::FUNCTION && i == 1 && j == 0) {
     bytecode_append(op::jump, SENTINEL);
     indent_type_pending = IndentType::FUNCTION;

     string name = token;
     addr address = writer;

     // name
     symbols[name].type = FUNCTION;
     symbols[name].address = address;
     cout << name << " function is written at bytecode [" << address << "]" << endl;

     // arguments
     address = slotter;
     u8 length = expression_ordered.size() - 2;
     addr slot_after = slotter + length + 1;
     for (s32 i = length; i >= 0; i--) {
      name = expression_ordered[i + 1];
      symbols[name].type = NUMBER;
      symbols[name].address = address + i;
      bytecode_append(op::storeto, address + i);
      cout << name << " is stored in " << address + i << endl;
     }

     slotter = slot_after;
    }
   }
  }

  // return
  if (is_return) {bytecode_append(op::subret, 0);}

  // if
  if (header_type == HeaderType::IF) {
   bytecode_append(op::jumz, SENTINEL);
   indent_type_pending = IndentType::IF;
  }

  // while
  if (header_type == HeaderType::WHILE) {
   bytecode_append(op::jumz, SENTINEL);
   indent_type_pending = IndentType::WHILE;
  }

  // declaration
  if (assign_type == AssignType::DECLARE) {
   string name = tokens[1];
   addr address = 0;

   switch (declare_style) {
    case DeclareStyle::VAR: {
     address = slotter++;
     symbols[name].type = NUMBER;
     symbols[name].address = address;
     bytecode_append(op::storeto, address);
     cout << name << " is stored in " << address << endl;
     break;
    }
    case DeclareStyle::STRING_FULL: {
     u32 length = tokens[3].size() - 2; // exclude quotes
     symbols[name].type = STRING;
     symbols[name].address = slotter - (length + 1);

     cout << name << " string is stored in " << symbols[name].address << " with length " << length << endl;
     break;
    }
    case DeclareStyle::STRING_SIZE: {
     address = slotter;
     s32 length = cast(s32, stof(tokens[2])); // truncate // WARNING: not expression!
     symbols[name].type = STRING;
     symbols[name].address = address;

     slotter += length;

     cout << name << " empty string is stored in " << address << " with length " << length << endl;
     break;
    }
    case DeclareStyle::STRIPE_FULL: {
     address = slotter;
     s32 count = tokens.size() - 3;
     symbols[name].type = STRIPE;
     symbols[name].address = address;

     slotter += count;

     for (s32 i = count - 1; i >= 0; i--) {
      bytecode_append(op::storeto, address + i);
     }

     cout << name << " stripe is stored in " << address << " with size " << count << endl;
     break;
    }
    case DeclareStyle::STRIPE_SIZE: {
     address = slotter;
     s32 count = cast(s32, stof(tokens[2])); // truncate // WARNING: not expression!
     symbols[name].type = STRIPE;
     symbols[name].address = address;

     slotter += count;

     cout << name << " empty stripe is stored in " << address << " with size " << count << endl;
     break;
    }
    default: {break;}
   }
  }

  // assignment
  if (assign_type == AssignType::SET) {
   switch (set_style) {
    case SetStyle::VAR: {
     string name = tokens[0];
     addr address = symbols[name].address;
     bytecode_append(op::storeto, address);
     break;
    }
    case SetStyle::STRIPE: {
     bytecode_append(op::poke, op::nop);
     break;
    }
    default: {break;}
   }
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
  name == "jump"   || name == "jumz" || name == "junz" ||
  name == "subgo"
 );
 string value = "";
 if (has_operand) {
  if (operand == SENTINEL) {value = "???";}
  else {
   value = to_string(operand);
   if (name == "pop" || name == "push") {
    value += " (" + utility::string_no_trailing(fpu::unscale(operand)) + ")";
    elem operand_unpacked = fpu::unpack(operand);
    if (operand_unpacked >= 32 && operand_unpacked <= 126) { // printable ASCII
     value += " \"" + string(1, cast(char, operand_unpacked)) + "\"";
    }
   }
   if (name == "takefrom" || name == "storeto") {
    for (hash_map<string, SymbolData>::iterator it = symbols.begin(); it != symbols.end(); ++it) {
     if (cast(elem, it->second.address) == operand) {value += " (" + it->first + ")"; break;}
    }
   }
  }
 }

 cout << "[" << writer << "] " << name << "\t[" << writer + 1 << "] " << value << endl;

 bytecode[writer++] = opcode;
 bytecode[writer++] = operand;
}

static vector<string> breakdown(const string& expression) {
 vector<string> tokens;
 string token;
 bool in_quote = false;

 for (u32 i = 0; i < expression.size(); i++) {
  char c = expression[i];

  // quote (with backslash escape)
  if (c == '"') {
   u8 bs = 0;
   for (s32 pos = i - 1; pos >= 0 && expression[pos] == '\\'; pos--) bs++;
   if (bs % 2 == 0) {
    if (in_quote) {token += c;}
    if (!token.empty()) {tokens.push_back(token); token.clear();}
    if (!in_quote) {token += c;}
    in_quote = !in_quote;
    continue;
   }
  }
  if (in_quote) {token += c; continue;}

  // hex
  if (c == '0' && (i + 1 < expression.size()) && (expression[i+1] == 'x' || expression[i+1] == 'X')) {
   string hexnum = "0x";
   i += 2;
   bool has_dot = false;
   while (i < expression.size() && (isxdigit(expression[i]) || (!has_dot && expression[i] == '.'))) {
    if (expression[i] == '.') {
     has_dot = true;
    }
    hexnum += expression[i++];
   }
   tokens.push_back(hexnum);
   i--; // adjust for loop increment
   continue;
  }

  // operators
  bool operator_matched = false;
  for (u8 len : {2, 1}) {
   string candidate = expression.substr(i, len);
   if (math_list_operations_bracket.count(candidate)) {
    if (!token.empty()) {
     tokens.push_back(token);
     token.clear();
    }
    tokens.push_back(candidate);
    i += len - 1;
    operator_matched = true;
    break;
   }
  }
  if (operator_matched) {continue;}

  // comma
  if (c == ',') {
   if (!token.empty()) {
    tokens.push_back(token);
    token.clear();
   }
   tokens.push_back(",");
   continue;
  }

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
 }

 return tokens;
}

static u8 precedence(const string& op) {
#define OP(sym, code, prec) if (op == sym) {return prec;}
 SORTED_OPERATORS
#undef OP
 if (op == OPERATOR_OFFSET) {return OFFSET;}
 return BASE;
}

static vector<string> postfix(const vector<string>& tokens) {
 vector<string> output;
 vector<string> operator_stack;

 for (u32 i = 0; i < tokens.size(); i++) {
  string token = tokens[i];

  // flush when comma
  if (token == ",") {
   while (!operator_stack.empty() && operator_stack.back() != "(") {
    output.push_back(operator_stack.back());
    operator_stack.pop_back();
   }
   continue;
  }

  // unary "-"
  if (token == "-" && (i == 0 || math_list_operations.count(tokens[i-1]) || tokens[i-1] == "(" || tokens[i-1] == ",")) {
   // merge with number
   if (i + 1 < tokens.size() && utility::is_number(tokens[i + 1])) {
    output.push_back("-" + tokens[++i]); // skip next token
    continue;
   }
   // convert to "neg"
   token = "neg";
  }

  // function
  bool is_opcode = opcode_get(token.c_str()) != op::nop;
  bool is_function = symbols.count(token) && symbols[token].type == FUNCTION;
  if (is_opcode || is_function) {
   operator_stack.push_back(token);
   continue;
  }

  if (math_list_operations.count(token)) {
   while (
    !operator_stack.empty()
    && math_list_operations.count(operator_stack.back())
    && (
     (precedence(operator_stack.back()) > precedence(token))
     || (
      precedence(operator_stack.back()) == precedence(token)
      && true // is_left_assoc(token) // all basic ops are left-associative
     )
    )
   ) {
    output.push_back(operator_stack.back());
    operator_stack.pop_back();
   }
   operator_stack.push_back(token);
  }

  else if (token == "(") {
   operator_stack.push_back(token);
  }

  else if (token == ")") {
   while (!operator_stack.empty() && operator_stack.back() != "(") {
    output.push_back(operator_stack.back());
    operator_stack.pop_back();
   }
   if (!operator_stack.empty() && operator_stack.back() == "(") {
    operator_stack.pop_back();
   }

   // emit function after its arguments
   if (!operator_stack.empty()) {
    is_opcode = opcode_get(operator_stack.back().c_str()) != op::nop;
    is_function = symbols.count(operator_stack.back()) && symbols[operator_stack.back()].type == FUNCTION;
    if (is_opcode || is_function) {
     output.push_back(operator_stack.back());
     operator_stack.pop_back();
    }
   }
  }

  // number or variable
  else {output.push_back(token);}
 }

 // pop any remaining ops
 while (!operator_stack.empty()) {
  output.push_back(operator_stack.back());
  operator_stack.pop_back();
 }

 return output;
}