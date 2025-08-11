#include "core/memory.hpp"
#include "core/utility.hpp"
#include "module/interpreter.hpp"
#include "module/opcode.hpp"

// TODO:
// handle warning of exception, overflow, etc

constexpr str SYMBOL_STRING_PREFIX = "&:";
constexpr str OPERATOR_OFFSET = "#";
constexpr u8 OPERATOR_COMMENT = '$';

hash_map<string, Redirect> redirect;

enum class Precedence : u8 {
 Base = 0,
 Logic = 0,
 Compare = 1,
 Shift = 2,
 Add = 3,
 Multiply = 4,
 Offset = 5,
 Unary = 6,
};

#define SORTED_OPERATORS \
 OP("==", op::eq, Precedence::Compare) \
 OP("!=", op::neq, Precedence::Compare) \
 OP("<=", op::leq, Precedence::Compare) \
 OP(">=", op::geq, Precedence::Compare) \
 OP("&&", op::band, Precedence::Logic) \
 OP("||", op::bor, Precedence::Logic) \
 OP("~~", op::bnot, Precedence::Unary) \
 OP("<<", op::bshl, Precedence::Shift) \
 OP(">>", op::bshr, Precedence::Shift) \
 OP("+", op::add, Precedence::Add) \
 OP("-", op::sub, Precedence::Add) \
 OP("*", op::mul, Precedence::Multiply) \
 OP("/", op::div, Precedence::Multiply) \
 OP("<", op::lt, Precedence::Compare) \
 OP(">", op::gt, Precedence::Compare) \
 OP("&", op::land, Precedence::Logic) \
 OP("|", op::lor, Precedence::Logic) \
 OP("!", op::lnot, Precedence::Unary)

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

static void debug_opcode(u8 opcode, elem operand, addr ticker);
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

  enum class HeaderType : u8 {None, If, While, Else, Function};
  HeaderType header_type = HeaderType::None;

  enum class AssignType : u8 {None, Declare, Set};
  AssignType assign_type = AssignType::None;

  enum class DeclareStyle : u8 {None, Variable, StringFull, StringSize, StripeFull, StripeSize};
  DeclareStyle declare_style = DeclareStyle::None;

  enum class SetStyle : u8 {None, Variable, String, Stripe};
  SetStyle set_style = SetStyle::None;

  bool is_expression = (find(tokens.begin(), tokens.end(), "=") == tokens.end());

  bool is_return = tokens[0] == "return";

  // indent
  if (indent > scope::previous) {
   if (indent - scope::previous > 1) {cout << "caution: too much indent" << endl;}
   for (u8 indent_level = scope::previous + 1; indent_level <= indent; ++indent_level) {
    cout << ">>>>" << endl;

    if (scope::last_line_scope_set != scope::Type::Function) { // if, while, and else
     const elem last_opcode = bytecode[writer - 2];
     if (last_opcode != op::jump && last_opcode != op::jumz && last_opcode != op::junz) {
      cout << "caution: last opcode before indent is not jump/jumz/junz" << endl;
     }
    }

    scope::stack.push_back({scope::last_jump_operand, scope::last_line_start, scope::last_line_scope_set});
    scope::last_line_scope_set = scope::Type::Generic;
    cout << "indent stack added with jump operand at " << scope::last_jump_operand << endl;
   }
  }

  // dedent
  if (indent < scope::previous) {
   for (u8 indent_level = scope::previous; indent_level > indent; --indent_level) {
    cout << "<<<<" << endl;

    // only handle else because at "else" its a single line dedent with only one keyword
    // so this handled earlier before patching the bytecode jump operand with writer
    if (tokens[0] == "else") {
     header_type = HeaderType::Else;

     bytecode_append(op::jump, SENTINEL);
     scope::last_jump_operand = writer - 1;
     scope::last_line_scope_set = scope::Type::Else;
    }

    const scope::Frame& frame = scope::stack.back();

    if (frame.type == scope::Type::While) {
     bytecode_append(op::jump, frame.header_start); // next opcode
    }

    bytecode[frame.jump_operand] = writer;
    cout << "patching jump operand at " << frame.jump_operand << " to address " << writer << endl;

    scope::stack.pop_back();
   }
  }
  scope::previous = indent;

  cout << "\n";
  for (const string& t : tokens) {cout << t << " ";}
  cout << endl;

  if (tokens[0] == "if") {header_type = HeaderType::If;}
  if (tokens[0] == "while") {header_type = HeaderType::While;}
  if (tokens[0] == "func") {header_type = HeaderType::Function;}
  scope::last_line_start = writer;

  // else
  // already handled earlier in dedent
  if (header_type == HeaderType::Else) {return;}

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
   assign_type = AssignType::Declare;
   declare_style = DeclareStyle::Variable;
  }
  if (tokens[0] == "string" && tokens[2] == "=" && tokens.size() == 4 && tokens[3].size() >= 2 && tokens[3].front() == '"' && tokens[3].back() == '"') {
   assign_type = AssignType::Declare;
   declare_style = DeclareStyle::StringFull;
  }
  if (tokens[0] == "string" && tokens.size() == 3 && utility::is_number(tokens[2])) {
   assign_type = AssignType::Declare;
   declare_style = DeclareStyle::StringSize;
  }
  if (tokens[0] == "stripe" && tokens[2] == "=" && tokens.size() >= 4) {
   assign_type = AssignType::Declare;
   declare_style = DeclareStyle::StripeFull;
  }
  if (tokens[0] == "stripe" && tokens.size() == 3 && utility::is_number(tokens[2])) {
   assign_type = AssignType::Declare;
   declare_style = DeclareStyle::StripeSize;
  }
  if (tokens.size() == 3 && tokens[1] == "=") {
   assign_type = AssignType::Set;
   set_style = SetStyle::Variable;

   u64 has_hash = tokens[0].find('#');
   if (has_hash != string::npos && symbol::exist(tokens[0].substr(0, has_hash))) {set_style = SetStyle::Stripe;}
   if (symbol::exist(tokens[0]) && symbol::get(tokens[0]).type == symbol::Type::String) {set_style = SetStyle::String;}
  }

  // token breakdown
  for (u32 i = 0; i < tokens.size(); i++) {
   const string& token = tokens[i];
   if (token == "=") {is_expression = true; continue;}

   vector<string> expression_breaked = breakdown(token);
   // cout << "BREAK: "; for (const string &token : expression_breaked) {cout << "'" << token << "' ";} cout << endl;
   vector<string> expression_ordered = postfix(expression_breaked);
   // cout << "SORT: "; for (const string &token : expression_ordered) {cout << "'" << token << "' ";} cout << endl;
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
     if (symbol::exist(name)) {
      cout << "string already exist in " << symbol::get(name).address << " is written \"" << content << "\"" << endl;
      continue;
     }

     addr address = slotter;
     u32 length = content.size();
     addr slot_after = slotter + length + 1;
     if (set_style == SetStyle::String) {
      address = symbol::get(tokens[0]).address;
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

     if (declare_style == DeclareStyle::StringFull || set_style == SetStyle::String) {continue;}
     // hidden declare
     symbol::table.push_back({name, slotter - (length + 1), symbol::Type::String});

     cout << "string hidden declare in " << symbol::get(name).address << " is written \"" << content << "\"" << endl;

     // also push to stack for other use
     bytecode_append(op::push, fpu::pack(symbol::get(name).address));
    }

    // number
    // hardest part to document
    // all detail of rounding, casting, data type is important
    if ((utility::is_number(token) || utility::is_hex(token)) && declare_style != DeclareStyle::StripeSize && declare_style != DeclareStyle::StringSize) {
     double value = utility::is_hex(token) ? utility::hex_to_number(token) : stod(token);
     bytecode_append(op::push, cast(elem, cast(s64, round(fpu::scale(value))))); // rounded fixed point
    }

    // symbol
    if (symbol::exist(token)) {
     if (header_type == HeaderType::Function) {continue;}
     const symbol::Data& symbol = symbol::get(token);
     switch (symbol.type) {
      case symbol::Type::Number: {
       if (assign_type == AssignType::Set && set_style == SetStyle::Variable && !is_expression && token == tokens[0]) {break;}
       bytecode_append(op::takefrom, symbol.address);
       break;
      }
      case symbol::Type::String: {
       if (assign_type == AssignType::Set && set_style == SetStyle::String && !is_expression && token == tokens[0]) {break;}
       bytecode_append(op::push, fpu::pack(symbol.address));
       break;
      }
      case symbol::Type::Stripe: {
       bytecode_append(op::push, fpu::pack(symbol.address));
       break;
      }
      case symbol::Type::Function: {
       bytecode_append(op::subgo, symbol.address);
       break;
      }
      default: {break;}
     }
    }

    // "#"
    if (token == OPERATOR_OFFSET) {
     bytecode_append(op::add, op::nop);
     if (assign_type == AssignType::Set && set_style == SetStyle::Stripe && !is_expression && j == expression_ordered.size() - 1) {continue;}
     bytecode_append(op::peek, op::nop);
    }

    // math operations
    if (math_opcodes.count(token)) {
     bytecode_append(math_opcodes.at(token), op::nop);
    }

    // function
    // this is oneshot, inside breakdown for function name extraction
    if (header_type == HeaderType::Function && i == 1 && j == 0) {
     scope::last_jump_operand = writer + 1;
     bytecode_append(op::jump, SENTINEL);
     scope::last_line_scope_set = scope::Type::Function;

     string name = token;
     addr address = writer;

     // name
     symbol::table.push_back({name, address, symbol::Type::Function});
     cout << name << " function is written at bytecode [" << address << "]" << endl;

     // arguments
     address = slotter;
     u8 length = expression_ordered.size() - 2;
     addr slot_after = slotter + length + 1;
     for (s32 i = length; i >= 0; i--) {
      name = expression_ordered[i + 1];
      symbol::table.push_back({name, address + i, symbol::Type::Number});
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
  if (header_type == HeaderType::If) {
   bytecode_append(op::jumz, SENTINEL);
   scope::last_jump_operand = writer - 1;
   scope::last_line_scope_set = scope::Type::If;
  }

  // while
  if (header_type == HeaderType::While) {
   bytecode_append(op::jumz, SENTINEL);
   scope::last_jump_operand = writer - 1;
   scope::last_line_scope_set = scope::Type::While;
  }

  // declaration
  if (assign_type == AssignType::Declare) {
   string name = tokens[1];
   addr address = SENTINEL;

   switch (declare_style) {
    case DeclareStyle::Variable: {
     address = slotter++;
     symbol::table.push_back({name, address, symbol::Type::Number});
     bytecode_append(op::storeto, address);
     cout << name << " is stored in " << address << endl;
     break;
    }
    case DeclareStyle::StringFull: {
     u32 length = tokens[3].size() - 2; // exclude quotes
     symbol::table.push_back({name, slotter - (length + 1), symbol::Type::String});

     cout << name << " string is stored in " << symbol::get(name).address << " with length " << length << endl;
     break;
    }
    case DeclareStyle::StringSize: {
     address = slotter;
     s32 length = cast(s32, stof(tokens[2])); // truncate // WARNING: not expression!
     symbol::table.push_back({name, address, symbol::Type::String});

     slotter += length;

     cout << name << " empty string is stored in " << address << " with length " << length << endl;
     break;
    }
    case DeclareStyle::StripeFull: {
     address = slotter;
     s32 count = tokens.size() - 3;
     symbol::table.push_back({name, address, symbol::Type::Stripe});

     slotter += count;

     for (s32 i = count - 1; i >= 0; i--) {
      bytecode_append(op::storeto, address + i);
     }

     cout << name << " stripe is stored in " << address << " with size " << count << endl;
     break;
    }
    case DeclareStyle::StripeSize: {
     address = slotter;
     s32 count = cast(s32, stof(tokens[2])); // truncate // WARNING: not expression!
     symbol::table.push_back({name, address, symbol::Type::Stripe});

     slotter += count;

     cout << name << " empty stripe is stored in " << address << " with size " << count << endl;
     break;
    }
    default: {break;}
   }
  }

  // assignment
  if (assign_type == AssignType::Set) {
   switch (set_style) {
    case SetStyle::Variable: {
     string name = tokens[0];
     addr address = symbol::get(name).address;
     bytecode_append(op::storeto, address);
     break;
    }
    case SetStyle::Stripe: {
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

  // debug_opcode(opcode, operand, counter);

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
 debug_opcode(opcode, operand, writer);
 bytecode[writer++] = opcode;
 bytecode[writer++] = operand;
}

static void debug_opcode(u8 opcode, elem operand, addr ticker) {
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
    for (u32 i = 0; i < symbol::table.size(); i++) {
     if (cast(elem, symbol::table[i].address) == operand) {
      value += " (" + symbol::table[i].name + ")";
      break;
     }
    }
   }
  }
 }

 cout << "[" << ticker << "] " << name << "\t[" << ticker + 1 << "] " << value << endl;
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
  if (isdigit(c) || (c == '.' && !token.empty() && all_of(token.begin(), token.end(), cast(int(*)(int), isdigit)) && token.find('.') == string::npos)) {
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

static Precedence precedence(const string& op) {
#define OP(sym, code, prec) if (op == sym) {return prec;}
 SORTED_OPERATORS
#undef OP
 if (op == OPERATOR_OFFSET) {return Precedence::Offset;}
 return Precedence::Base;
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
  bool is_function = symbol::exist(token) && symbol::get(token).type == symbol::Type::Function;
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
    is_function = symbol::exist(operator_stack.back()) && symbol::get(operator_stack.back()).type == symbol::Type::Function;
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