#include "core/interpreter.hpp"
#include "core/constant.hpp"
#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"
#include "core/utility.hpp"

// TODO:
// handle warning of exception, overflow, etc


namespace interpreter {
 constexpr str TOKEN_TAG = "$"; // token tagged with this symbol can only be generated internally
 constexpr str SYMBOL_STRING_PREFIX = "str:";
 constexpr str ADDRESS_OF = "@";
 constexpr str ARGUMENT_OPTIONAL = "="; // cannot cnage to colon `:` because it would bugs header end symbol which supposed to be unused symbol aaarrrggghhh!!!

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
  OP("%", op::mod, Precedence::Multiply) \
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
 };

 static const hash_set<string> math_list_operations_bracket = {
  #define OP(sym, code, prec) sym,
  SORTED_OPERATORS
  #undef OP
  "(",
  ")",
  "[",
  "]",
 };

 static void debug_opcode(octo opcode, fpu operand, addr ticker) {
  string name = opcode::name(opcode);
  if (name.length() < 4) {name += string(4 - name.length(), ' ');}

  bool has_operand = (
   name == "pop" || name == "push" ||
   name == "takefrom" || name == "storeto" ||
   name == "jump" || name == "jumz" || name == "junz" ||
   name == "subgo" || name == "call"
  );
  string value = "";
  if (has_operand) {
   if (operand == memory::vm::ram_global::constant::sentinel) {value = "!UNPATCHED!";}
   else if (operand == SIGNATURE) {value = "(unused)";}
   else {
    value = to_string(operand.value);
    if (name == "push") {
     value += " (" + utility::string_no_trailing(operand) + ")";

     // printable ASCII
     value += " \"";
     for (u8 i = 0; i < 4; i++) {value += cast(char, (operand.value >> (i * 8)) & 0xFF);}
     value += "\"";
    }
    else if (name == "takefrom" || name == "storeto") {
     for (u32 i = 0; i < symbol::table.size(); i++) {
      if (symbol::table[i].address == cast(addr, operand)) {
       value += " (" + symbol::table[i].name + ")";
       break;
      }
     }
    }
    else if (name == "call") {
     value += " (" + module::get_name(operand) + ")";
    }
    else if (name == "subgo") {
     for (s32 i = symbol::table.size() - 1; i >= 0; i--) {
      if (symbol::table[i].type == symbol::Type::Function && symbol::table[i].address == cast(addr, operand)) {
       value += " (" + symbol::table[i].name + ")";
       break;
      }
     }
    }
   }
  }

  cout << "[" << ticker << "] " << name << "\t[" << ticker + 1 << "] " << value << endl;
 }

 void bytecode_append(octo opcode, fpu operand) {
  using namespace memory::vm::process::app;
  using namespace ram_local;
  debug_opcode(opcode, operand, writer);
  bytecode[writer] = opcode;
  memory::unaligned_32_write(bytecode + writer + 1, operand.value);
  writer += 5;
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

   // hex and bin
   if (c == '0' && (i + 1 < expression.size())) {
    char p = expression[i + 1];
    bool is_hex = (p == 'x' || p == 'X');
    bool is_bin = (p == 'b' || p == 'B');
    if (is_hex || is_bin) {
     string num = is_hex ? "0x" : "0b";
     i += 2;
     bool has_dot = false;
     bool has_digit = false;
     while (i < expression.size()) {
      char d = expression[i];
      bool digit_ok = is_hex ? isxdigit(d) : (d == '0' || d == '1');
      if (digit_ok) {has_digit = true; num += d; i++; continue;}
      if (d == '.' && !has_dot) {has_dot = true; num += d; i++; continue;}
      break;
     }
     if (has_digit) {tokens.push_back(num);}
     else {tokens.push_back("0"); cout << "error: incomplete " << (is_hex ? "hex" : "bin") << " literal" << endl;}
     i--; // adjust for loop increment
     continue;
    }
   }

   // addressof
   if (c == ADDRESS_OF[0] && token.empty()) {
    token += c;
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

   // variable (alphabet, _, or optional argument assignment)
   // WARNING: this is not the elegant way to handle argument assignment symbol
   // for now it is a hacky way to make optional argument compilable
   if (isalpha(c) || c == '_' || c == ARGUMENT_OPTIONAL[0]) {
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
  return Precedence::Base;
 }

 static void fold(vector<string>& output, vector<string>& stash, const string& incoming_token = "", const string& stop_at = "") {
  while (
   output.size() >= 2
   && utility::is_number(output[output.size() - 1])
   && utility::is_number(output[output.size() - 2])
   && !stash.empty()
   && (incoming_token.empty() || precedence(stash.back()) >= precedence(incoming_token))
   && (stop_at.empty() || stash.back() != stop_at)
  ) {
   fpu b = fpu(stod(output.back())); output.pop_back();
   fpu a = fpu(stod(output.back())); output.pop_back();
   octo opcode = math_opcodes.at(stash.back()); stash.pop_back();

   fpu result;
   #define OP(name, expr) case op::name: {result = expr; break;}
   switch (opcode) {
    OP(eq, a == b ? 1 : 0)
    OP(neq, a != b ? 1 : 0)
    OP(leq, a <= b ? 1 : 0)
    OP(geq, a >= b ? 1 : 0)
    OP(band, a & b)
    OP(bor, a | b)
    OP(bnot, ~a)
    OP(bshl, a << cast(s32, b))
    OP(bshr, a >> cast(s32, b))
    OP(add, a + b)
    OP(sub, a - b)
    OP(mul, a * b)
    OP(div, b ? a / b : fpu(0))
    OP(mod, b ? a - fpu(floor(cast(double, a / b))) * b : fpu(0))
    OP(lt, a < b ? 1 : 0)
    OP(gt, a > b ? 1 : 0)
    OP(land, (a && b) ? 1 : 0)
    OP(lor, (a || b) ? 1 : 0)
    OP(lnot, !a ? 1 : 0)
    default: {result = 0; break;}
   }
   #undef OP

   output.push_back(utility::string_no_trailing(result));
  }
 }

 static vector<string> postfix(const vector<string>& tokens) {
  vector<string> output;
  vector<string> stash;
  vector<u8> paren_args_count;

  for (u32 i = 0; i < tokens.size(); i++) {
   string token = tokens[i];

   // flush when comma
   if (token == ",") {
    while (!stash.empty() && stash.back() != "(") {
     output.push_back(stash.back());
     stash.pop_back();
    }
    if (!paren_args_count.empty()) {paren_args_count.back()++;}
    continue;
   }

   // hex/bin to decimal
   if (utility::is_hex(token)) {token = to_string(utility::hex_to_number(token));}
   if (utility::is_bin(token)) {token = to_string(utility::bin_to_number(token));}

   // unary "-"
   if (token == "-" && (i == 0 || math_list_operations.count(tokens[i-1]) || tokens[i-1] == "(" || tokens[i-1] == ",")) {
    // merge with number
    if (i + 1 < tokens.size() && utility::is_number(tokens[i + 1])) {token = "-" + tokens[++i];}
    else {token = "neg";}
   }

   // function
   bool is_opcode = opcode::exist(token);
   bool is_function = symbol::exist(token) && symbol::get(token).type == symbol::Type::Function;
   bool is_module = module::exist(token);
   if (is_opcode || is_function || is_module) {
    stash.push_back(token);
    continue;
   }

   if (math_list_operations.count(token)) {
    // fold constant
    fold(output, stash, token);

    // defer operator
    while (
     !stash.empty()
     && math_list_operations.count(stash.back())
     && precedence(stash.back()) > precedence(token)
    ) {
     output.push_back(stash.back());
     stash.pop_back();
    }
    stash.push_back(token);
   }

   else if (token == "(" || token == "[") {
    stash.push_back(token);
    if (token == "(") {paren_args_count.push_back(0);}
   }

   else if (token == ")" || token == "]") {
    string pair = (token == ")") ? "(" : "[";

    while (!stash.empty() && stash.back() != pair) {
     fold(output, stash, "", pair);
     if (!stash.empty() && stash.back() != pair) {
      output.push_back(stash.back());
      stash.pop_back();
     }
    }
    if (!stash.empty() && stash.back() == pair) {
     stash.pop_back();
    }

    // tag function token with the amount of provided argument
    if (token == ")" && !stash.empty()) {
     is_opcode = opcode::exist(stash.back());
     is_function = symbol::exist(stash.back()) && symbol::get(stash.back()).type == symbol::Type::Function;
     is_module = module::exist(stash.back());

     if (is_opcode || is_function || is_module) {
      if (!paren_args_count.empty()) {
       output.push_back(stash.back() + TOKEN_TAG + to_string(cast(u32, paren_args_count.back())));
       paren_args_count.pop_back();
      }
      stash.pop_back();
     }

     if (!paren_args_count.empty() && paren_args_count.back() == 0) {paren_args_count.back() = 1;}
    }

    // emit offset operator after close bracket
    else if (token == "]") {
     output.push_back(TOKEN_TAG + string("offset"));
     for (const string& t : output) {if (t[0] == ADDRESS_OF[0]) {output.back() += ADDRESS_OF; break;}}
    }
   }

   // number or variable
   else {
    output.push_back(token);
    if (!paren_args_count.empty() && paren_args_count.back() == 0) {paren_args_count.back() = 1;}
   }
  }

  // pop any remaining ops
  while (!stash.empty()) {
   fold(output, stash);
   if (!stash.empty()) {
    output.push_back(stash.back());
    stash.pop_back();
   }
  }

  return output;
 }

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
   if (!in_quote && c == '#') {break;}

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
  using namespace memory::vm::process::app;
  using namespace ram_local;

  const u8& indent = line.indent;

  // AST-like token
  vector<vector<string>> tokens;
  for (const string& token : line.tokens) {
   vector<string> breaked = breakdown(token);
   vector<string> postfixed = postfix(breaked);
   tokens.push_back(postfixed);

   // cout << "/: "; for (const string& t : breaked) {cout << "'" << t << "' ";} cout << endl;
   // cout << ">: "; for (const string& t : postfixed) {cout << "'" << t << "' ";} cout << endl;
  }

  if (tokens.empty()) {return;}

  // debug print formatted token
  for (u32 i = 0; i < tokens.size(); i++) {
   cout << "[";
   for (u32 j = 0; j < tokens[i].size(); j++) {
    cout << "'" << tokens[i][j] << "'";
    if (j < tokens[i].size() - 1) {cout << " ";}
   }
   cout << "]";
   if (i < tokens.size() - 1) {cout << " ";}
  }
  cout << endl;

  // flag

  enum class HeaderType : u8 {None, If, While, Else, Function};
  HeaderType header_type = HeaderType::None;

  enum class AssignType : u8 {None, Declare, Set};
  AssignType assign_type = AssignType::None;

  enum class DeclareStyle : u8 {None, Variable, StringFull, StringSize, StripeFull, StripeSize};
  DeclareStyle declare_style = DeclareStyle::None;

  enum class SetStyle : u8 {None, Variable, String, Stripe};
  SetStyle set_style = SetStyle::None;

  bool is_expression = true;
  for (const vector<string>& token : tokens) {
   if (find(token.begin(), token.end(), "=") != token.end()) {is_expression = false; break;}
  }

  bool is_return = tokens[0][0] == "return";

  // indent
  if (indent > scope::previous) {
   if (indent - scope::previous > 1) {cout << "caution: too much indent" << endl;}
   for (u8 indent_level = scope::previous + 1; indent_level <= indent; ++indent_level) {
    cout << ">>>>" << endl;

    if (scope::last_line_scope_set != scope::Type::Function) { // if, while, and else
     const octo last_opcode = bytecode[writer - fpu(5)];
     if (last_opcode != op::jump && last_opcode != op::jumz && last_opcode != op::junz) {
      cout << "caution: last opcode before indent is not jump/jumz/junz" << endl;
     }
    }

    scope::Frame frame;
    frame.jump_operand = scope::last_jump_operand;
    frame.header_start = scope::last_line_start;
    frame.type = scope::last_line_scope_set;
    frame.symbol_start = symbol::table.size();
    frame.stack_start = stacker;
    scope::stack.push_back(frame);

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
    if (tokens[0][0] == "else") {
     header_type = HeaderType::Else;

     bytecode_append(op::jump, memory::vm::ram_global::constant::sentinel);
     scope::last_jump_operand = writer - fpu(4);
     scope::last_line_scope_set = scope::Type::Else;
    }

    const scope::Frame& frame = scope::stack.back();

    symbol::table.resize(frame.symbol_start);
    stacker = frame.stack_start;

    switch (frame.type) {
     case scope::Type::While: {
      bytecode_append(op::jump, frame.header_start); // next opcode

      // patch break
      for (addr patch_addr : frame.break_unpatched) {
       memory::unaligned_32_write(bytecode + patch_addr, writer.value);
       cout << "patching break at " << patch_addr << " to " << cast(addr, writer) << endl;
      }
      break;
     }
     case scope::Type::Function: {
      if (bytecode[writer - fpu(5)] != op::subret) { // last opcode
       cout << "implicit return emitted at " << cast(addr, writer) << endl;
       bytecode_append(op::subret, SIGNATURE);
      }
      break;
     }
     default: {break;}
    }

    memory::unaligned_32_write(bytecode + frame.jump_operand, writer.value);
    cout << "patching jump operand at " << frame.jump_operand << " to address " << cast(addr, writer) << endl;

    scope::stack.pop_back();
   }
  }
  scope::previous = indent;

  if (tokens[0][0] == "if") {header_type = HeaderType::If;}
  if (tokens[0][0] == "while") {header_type = HeaderType::While;}
  if (tokens[0][0] == "func") {header_type = HeaderType::Function;}
  scope::last_line_start = writer;

  // else
  // already handled earlier in dedent
  if (header_type == HeaderType::Else) {return;}

  if (tokens[0][0] == "break" || tokens[0][0] == "continue") {
   for (s32 i = scope::stack.size() - 1; i >= 0; i--) {
    if (scope::stack[i].type != scope::Type::While) {continue;}

    if (tokens[0][0] == "break") {
     bytecode_append(op::jump, memory::vm::ram_global::constant::sentinel);
     addr patch_addr = cast(addr, writer) - 4;
     scope::stack[i].break_unpatched.push_back(patch_addr);
     cout << "break stores unpatched jump at " << patch_addr << endl;
    }
    else { // continue
     bytecode_append(op::jump, scope::stack[i].header_start);
     cout << "continue jumps to while header at " << scope::stack[i].header_start << endl;
    }

    return;
   }
   cout << "error: " << tokens[0][0] << " outside of while loop" << endl;
   return;
  }

  // set type
  if (tokens[0][0] == "var" && tokens[2][0] == "=" && tokens.size() == 4) {
   assign_type = AssignType::Declare;
   declare_style = DeclareStyle::Variable;
  }
  if (tokens[0][0] == "string" && tokens[2][0] == "=" && tokens.size() == 4 && tokens[3][0].size() >= 2 && tokens[3][0].front() == '"' && tokens[3][0].back() == '"') {
   assign_type = AssignType::Declare;
   declare_style = DeclareStyle::StringFull;
  }
  if (tokens[0][0] == "string" && tokens.size() == 3 && tokens[2].size() == 1 && utility::is_number(tokens[2][0])) {
   assign_type = AssignType::Declare;
   declare_style = DeclareStyle::StringSize;
  }
  if (tokens[0][0] == "stripe" && tokens[2][0] == "=" && tokens.size() >= 4) {
   assign_type = AssignType::Declare;
   declare_style = DeclareStyle::StripeFull;
  }
  if (tokens[0][0] == "stripe" && tokens.size() == 3 && tokens[2].size() == 1 && utility::is_number(tokens[2][0])) {
   assign_type = AssignType::Declare;
   declare_style = DeclareStyle::StripeSize;
  }
  if (tokens.size() == 3 && tokens[1][0] == "=") {
   assign_type = AssignType::Set;
   set_style = SetStyle::Variable;

   if (tokens[0].size() == 1 && symbol::exist(tokens[0][0]) && symbol::get(tokens[0][0]).type == symbol::Type::String) {set_style = SetStyle::String;}

   if (tokens[0].size() >= 3 && tokens[0].back().find("$offset") == 0) {
    string base_name = tokens[0][0];
    if (symbol::exist(base_name) && symbol::get(base_name).type == symbol::Type::Stripe) {set_style = SetStyle::Stripe;}
   }
  }

  // token processing
  for (u32 i = 0; i < tokens.size(); i++) {
   const vector<string>& expression = tokens[i];
   if (expression.size() == 1 && expression[0] == "=") {is_expression = true; continue;}

   for (u32 j = 0; j < expression.size(); j++) {
    const string& token = expression[j];
    const u64 tag_pos = token.find(TOKEN_TAG);

    if (false) {}

    // tagged callable: module, function, or opcode
    else if (tag_pos && tag_pos != string::npos) { // ensure the tag is not prefix
     string name = token.substr(0, tag_pos);
     u8 args_count = stoi(token.substr(tag_pos + 1));

     bool is_module = module::exist(name);
     bool is_function = symbol::exist(name) && symbol::get(name).type == symbol::Type::Function;
     bool is_opcode = opcode::exist(name);

     u8 fn_args_count = 0;
     const vector<fpu>* fn_args_default = nullptr;

     if (is_module) {
      const module::Module& registered_module = module::table[module::get_index(name)];
      fn_args_count = registered_module.args_count;
      fn_args_default = &registered_module.args_default;
     }
     else if (is_function) {
      const symbol::Data& function_symbol = symbol::get(name);
      fn_args_count = function_symbol.args_count;
      fn_args_default = &function_symbol.args_default;
     }
     else if (is_opcode) {
      fn_args_count = opcode::args_count(name);
      static const vector<fpu> opcode_no_defaults = {};
      fn_args_default = &opcode_no_defaults;
     }

     const u8 fn_args_required = fn_args_count - fn_args_default->size();

     if (args_count < fn_args_required) {cout << "error: too few arguments for " << name << " (expected at least " << cast(u32, fn_args_required) << ", got " << cast(u32, args_count) << ")" << endl;}
     if (args_count > fn_args_count) {cout << "error: too many arguments for " << name << " (expected at most " << cast(u32, fn_args_count) << ", got " << cast(u32, args_count) << ")" << endl;}

     for (u8 i = args_count; i < fn_args_count; i++) {
      bytecode_append(op::push, (*fn_args_default)[i - fn_args_required]);
     }

     if (is_module) {bytecode_append(op::call, module::get_index(name));}
     else if (is_function) {bytecode_append(op::subgo, symbol::get(name).address);}
     else if (is_opcode) {bytecode_append(opcode::get(name), SIGNATURE);}
    }

    // string
    else if (token.front() == '"' && token.back() == '"') {
     string content = token.substr(1, token.size() - 2); // strip quotes
     string name = SYMBOL_STRING_PREFIX + content;
     if (symbol::exist(name)) {
      cout << "string already exist in " << symbol::get(name).address << " is written \"" << content << "\"" << endl;
      continue;
     }

     addr address = slotter;
     u32 length = content.size();
     u32 word_count = (length + 3) / 4;  // ceiling division of 4 character per fpu
     addr slot_after = cast(addr, slotter) + word_count + 1;
     if (set_style == SetStyle::String) {
      address = symbol::get(tokens[0][0]).address;
      slot_after = slotter;
     }
     bytecode_append(op::push, length);

     for (u32 word = 0; word < word_count; word++) {
      s32 packed_word = 0;
      for (u32 character = 0; character < 4; character++) {
       u32 char_index = word * 4 + character;
       if (char_index < length) {packed_word |= cast(s32, content[char_index]) << (character * 8);}
      }
      bytecode_append(op::push, fpu(packed_word, true));
     }

     for (s32 i = word_count; i >= 0; i--) {
      bytecode_append(op::storeto, address + i);
     }

     slotter = slot_after;

     if (declare_style == DeclareStyle::StringFull || set_style == SetStyle::String) {continue;}
     // hidden declare
     symbol::table.push_back({name, cast(addr, slotter) - (length + 1), symbol::Type::String});

     cout << "string hidden declare in " << symbol::get(name).address << " is written \"" << content << "\"" << endl;

     // also push to stack for other use
     bytecode_append(op::push, symbol::get(name).address);
    }

    // number
    // must be documented that the stored value is the rounded representation from the scaled value
    else if (utility::is_number(token) && declare_style != DeclareStyle::StripeSize && declare_style != DeclareStyle::StringSize) {
     bytecode_append(op::push, fpu(round(stod(token) * (1 << fpu::DECIMAL_WIDTH)), true));
    }

    // addressof
    else if (token[0] == ADDRESS_OF[0]) {
     string symbol_name = token.substr(1);
     if (!symbol::exist(symbol_name)) {
      cout << "error: symbol \"" << symbol_name << "\" not found" << endl;
      continue;
     }
     bytecode_append(op::push, symbol::get(symbol_name).address);
    }

    // symbol
    else if (symbol::exist(token)) {
     if (header_type == HeaderType::Function) {continue;}
     const symbol::Data& symbol = symbol::get(token);
     switch (symbol.type) {
      case symbol::Type::Number: {
       if (assign_type == AssignType::Set && set_style == SetStyle::Variable && !is_expression && tokens[0].size() == 1 && token == tokens[0][0]) {break;}
       bytecode_append(op::takefrom, symbol.address);
       break;
      }
      case symbol::Type::String: {
       if (assign_type == AssignType::Set && set_style == SetStyle::String && !is_expression && tokens[0].size() == 1 && token == tokens[0][0]) {break;}
       bytecode_append(op::push, symbol.address);
       break;
      }
      case symbol::Type::Stripe: {
       bytecode_append(op::push, symbol.address);
       break;
      }
      case symbol::Type::Function: {break;} // handled by tagged callable
      default: {break;}
     }
    }

    // stripe offset
    else if (token.find(TOKEN_TAG + string("offset")) == 0) {
     bytecode_append(op::add, SIGNATURE);
     if (assign_type == AssignType::Set && set_style == SetStyle::Stripe && !is_expression && j == expression.size() - 1) {continue;} // stripe assignment handles storage internally
     if (token.back() == ADDRESS_OF[0]) {continue;} // address semantics require raw address
     bytecode_append(op::get, SIGNATURE);
    }

    // math operations
    else if (math_opcodes.count(token)) {
     bytecode_append(math_opcodes.at(token), SIGNATURE);
    }

    // function
    // this is oneshot, inside breakdown for function name extraction
    else if (header_type == HeaderType::Function && i == 1 && j == 0) {
     scope::last_jump_operand = cast(addr, writer) + 1;
     bytecode_append(op::jump, memory::vm::ram_global::constant::sentinel);
     scope::last_line_scope_set = scope::Type::Function;

     string name = token;
     addr address = cast(addr, writer);

     // event loop
     if (name == "init") {memory::unaligned_32_write(bytecode + 1, fpu(address).value); cout << "event loop init is written at bytecode [" << address << "]" << endl;}
     else if (name == "step") {memory::unaligned_32_write(bytecode + 6, fpu(address).value); cout << "event loop step is written at bytecode [" << address << "]" << endl;}
     else if (name == "draw") {memory::unaligned_32_write(bytecode + 11, fpu(address).value); cout << "event loop draw is written at bytecode [" << address << "]" << endl;}

     // symbol
     symbol::table.push_back({name, address, symbol::Type::Function});
     cout << name << " function is written at bytecode [" << address << "]" << endl;
     u32 function_symbol_index = symbol::table.size() - 1;

     // arguments
     address = cast(addr, slotter);
     u8 args_count = expression.size() - 1;
     addr slot_after = cast(addr, slotter) + args_count;
     bool in_required = false;
     for (s32 i = args_count; i > 0; i--) {
      name = expression[i];

      // TODO:
      // the grace way to handle default value assignment is probably by making the argument assignment symbol a separate token
      // and then if encounter it then set the symbol of [token previous] with the value of [token next]
      // though it require space unaware token separation which means entirely more complex interpreter system
      u64 separator_pos = name.find(ARGUMENT_OPTIONAL);
      if (separator_pos != string::npos) { // args is optional
       string args_value_str = name.substr(separator_pos + 1);
       name = name.substr(0, separator_pos);
       if (name.empty()) {continue;}

       if (in_required) {
        cout << "error: required " << expression[i + 1] << " after optional " << name << " is not allowed" << endl;
        return;
       }

       if (!utility::is_number(args_value_str)) {
        cout << "error: default value for " << name << " must be numeric literal" << endl;
        continue;
       }

       symbol::table[function_symbol_index].args_default.insert(symbol::table[function_symbol_index].args_default.begin(), stod(args_value_str)); // insert in reverse order // WARNING: not expression!
       symbol::table[function_symbol_index].args_count++;
      }
      else { // args is required
       in_required = true;
       symbol::table[function_symbol_index].args_count++;
      }

      const addr args_slot = address + (i - 1);
      symbol::table.push_back({name, args_slot, symbol::Type::Number});
      bytecode_append(op::storeto, args_slot);
      cout << name << " is stored in " << args_slot << endl;
     }

     slotter = fpu(slot_after);
     break; // ensure no further token in line is processed
    }

    // invalid
    // terrible code, need proper non-keyword check
    // unfortunately, it seems like this is important and had to be this way
    // please reconsider
    else if (
     token != "var" &&
     token != "string" &&
     token != "stripe" &&
     token != "if" &&
     token != "while" &&
     token != "else" &&
     token != "func" &&
     token != "return" &&
     !(assign_type == AssignType::Declare && (i == 1 || (i == 2 && (declare_style == DeclareStyle::StringSize || declare_style == DeclareStyle::StripeSize))))
    )
    {
     cout << "error: invalid identifier \"" << token << "\"" << endl;
    }
   }
  }

  // return
  if (is_return) {bytecode_append(op::subret, SIGNATURE);}

  // if
  if (header_type == HeaderType::If) {
   bytecode_append(op::jumz, memory::vm::ram_global::constant::sentinel);
   scope::last_jump_operand = cast(addr, writer) - 4;
   scope::last_line_scope_set = scope::Type::If;
  }

  // while
  if (header_type == HeaderType::While) {
   bytecode_append(op::jumz, memory::vm::ram_global::constant::sentinel);
   scope::last_jump_operand = cast(addr, writer) - 4;
   scope::last_line_scope_set = scope::Type::While;
  }

  // declaration
  if (assign_type == AssignType::Declare) {
   string name = tokens[1][0];
   addr address = memory::vm::ram_global::constant::sentinel;

   switch (declare_style) {
    case DeclareStyle::Variable: {
     address = cast(addr, slotter++);
     symbol::table.push_back({name, address, symbol::Type::Number});
     bytecode_append(op::storeto, address);
     cout << name << " is stored in " << address << endl;
     break;
    }
    case DeclareStyle::StringFull: {
     u32 length = tokens[3][0].size() - 2; // exclude quotes
     symbol::table.push_back({name, cast(addr, slotter) - (((length + 3) / 4) + 1), symbol::Type::String}); // packed string size

     cout << name << " string is stored in " << symbol::get(name).address << " with length " << length << endl;
     break;
    }
    case DeclareStyle::StringSize: {
     if (tokens[2].size() != 1 || !utility::is_number(tokens[2][0])) {
      cout << "error: size must be constant expression" << endl;
      break;
     }
     address = cast(addr, slotter);
     s32 length = cast(s32, stod(tokens[2][0]));
     symbol::table.push_back({name, address, symbol::Type::String});

     slotter += fpu(cast(s32, ((length + 3) / 4) + 1)); // packed string size

     cout << name << " empty string is stored in " << address << " with length " << length << endl;
     break;
    }
    case DeclareStyle::StripeFull: {
     address = cast(addr, slotter);
     s32 count = tokens.size() - 3;
     symbol::table.push_back({name, address, symbol::Type::Stripe});

     slotter += fpu(count);

     for (s32 i = count - 1; i >= 0; i--) {
      bytecode_append(op::storeto, address + i);
     }

     cout << name << " stripe is stored in " << address << " with size " << count << endl;
     break;
    }
    case DeclareStyle::StripeSize: {
     if (tokens[2].size() != 1 || !utility::is_number(tokens[2][0])) {
      cout << "error: size must be constant expression" << endl;
      break;
     }
     address = cast(addr, slotter);
     s32 count = cast(s32, stod(tokens[2][0]));
     symbol::table.push_back({name, address, symbol::Type::Stripe});

     slotter += fpu(count);

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
     string name = tokens[0][0];
     addr address = symbol::get(name).address;
     bytecode_append(op::storeto, address);
     break;
    }
    case SetStyle::Stripe: {
     bytecode_append(op::set, SIGNATURE);
     break;
    }
    default: {break;}
   }
  }

  return;
 }

 void step() {
  using namespace memory::vm::process::app;
  using namespace ram_local;

  if (counter >= writer) {return;}

  octo opcode = bytecode[counter];
  fpu operand = fpu(memory::unaligned_32_read(bytecode + counter + 1), true);
  addr result = memory::vm::ram_global::constant::sentinel;

  // debug_opcode(opcode, operand, counter);

  switch (opcode) {
   #define OPI(hex, name) case op::name: result = opfunc::name(operand); break;
   #define OPC(hex, name, args) case op::name: result = opfunc::name(operand); break;
   OPCODES
   #undef OPI
   #undef OPC
  }

  if (result == cast(addr, memory::vm::ram_global::constant::sentinel)) {counter += 5;}
  else {counter = result;}
 }
}

namespace symbol {
 vector<Data> table;

 s32 get_index_reverse(const string& name) {
  for (u32 i = table.size(); i-- > 0;) {
   if (symbol::table[i].name == name) {return i;}
  }
  return -1;
 }

 bool exist(const string& name) {
  return get_index_reverse(name) >= 0;
 }

 Data& get(const string& name) {
  return table[get_index_reverse(name)];
 }
}

namespace scope {
 vector<Frame> stack;
 u8 previous = 0;

 addr last_jump_operand = memory::vm::ram_global::constant::sentinel;
 addr last_line_start = memory::vm::ram_global::constant::sentinel;
 Type last_line_scope_set = Type::Generic;
}