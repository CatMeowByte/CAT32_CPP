#include "core/constant.hpp"
#include "core/interpreter.hpp"
#include "core/opcode.hpp"
#include "core/utility.hpp"

namespace interpreter {
 static vector<vector<string>> breakdown(const string& line) {
  static const hash_set<char> boundaries = {'(', ')', '[', ']', ',', ':'};

  vector<vector<string>> output;
  vector<string> expression;
  string token;
  bool in_quote = false;
  u32 pos = 0;

  // indent
  while (pos < line.size() && line[pos] == ' ') {pos++;}
  output.push_back({to_string(pos)});

  // each character
  while (pos < line.size()) {
   char c = line[pos];

   // inside quote
   if (in_quote) {
    if (c == '"') {
     u8 backslash_count = 0;
     for (s32 i = pos - 1; i >= 0 && line[i] == '\\'; i--) {backslash_count++;}
     if (backslash_count % 2 == 0) {token += c; expression.push_back(token); token.clear(); in_quote = false;}
     else {token += c;}
    }
    else {token += c;}
    pos++;
    continue;
   }

   // push token
   if (!token.empty() && (
    c == ' ' || c == '#' || c == '='
    || boundaries.count(c)
    || metic::operations.count(string(1, c))
    || (pos + 1 < line.size() && metic::operations.count(line.substr(pos, 2)))
   )) {
    expression.push_back(token);
    token.clear();
    if (keywords_declaration.count(expression.back()) || keywords_control.count(expression.back())) {
     output.push_back(expression);
     expression.clear();
    }
   }

   if (false) {}

   // comment
   else if (c == '#') {break;}

   // quote
   else if (c == '"') {token += c; in_quote = true;}

   // hex and bin
   else if (c == '0' && token.empty() && pos + 1 < line.size() && (line[pos+1] == 'x' || line[pos+1] == 'X' || line[pos+1] == 'b' || line[pos+1] == 'B')) {
    bool is_hex = (line[pos+1] == 'x' || line[pos+1] == 'X');
    token += '0';
    token += line[pos+1];
    pos += 2;
    bool has_dot = false;
    while (pos < line.size()) {
     char d = line[pos];
     bool digit_valid = is_hex ? isxdigit(d) : (d == '0' || d == '1');
     if (digit_valid) {token += d; pos++;}
     else if (d == '.' && !has_dot) {has_dot = true; token += d; pos++;}
     else {break;}
    }
    expression.push_back(token);
    token.clear();
    continue;
   }

   // double character operator
   else if (pos + 1 < line.size() && metic::operations.count(line.substr(pos, 2))) {expression.push_back(line.substr(pos, 2)); pos++;}

   // single character operator
   else if (metic::operations.count(string(1, c))) {expression.push_back(string(1, c));}

   // boundary
   else if (boundaries.count(c)) {expression.push_back(string(1, c));}

   // equal
   else if (c == '=') {
    if (!expression.empty()) {output.push_back(expression); expression.clear();}
    expression.push_back(string(1, c));
    output.push_back(expression);
    expression.clear();
   }

   // space
   else if (c == ' ') {}

   // identifier
   else if (isalpha(c) || isdigit(c) || c == '_' || c == '.') {token += c;}

   pos++;
  }

  if (!token.empty()) {expression.push_back(token);}
  if (!expression.empty()) {output.push_back(expression);}

  return output;
 }

 static void substitute(vector<vector<string>>& tokens) {
  for (u32 i = 1; i < tokens.size(); i++) { // skip metadata token 0
   for (u32 j = 0; j < tokens[i].size(); j++) {
    // skip first token in lhs
    if (j == 0 && tokens.size() > 2) { if (
      (i == 2 && keywords_declaration.count(tokens[1][0])) // declare
      || (i == 1 && tokens[i].size() == 1 && tokens[2][0] == "=") // assign
     ) {continue;}
    }

    string& token = tokens[i][j];

    // hex/bin to decimal
    if (utility::is_hex(token)) {token = to_string(utility::hex_to_number(token));}
    if (utility::is_bin(token)) {token = to_string(utility::bin_to_number(token));}

    // constant
    if (symbol::exist(token) && symbol::get(token).type == symbol::Type::Constant) {
     token = utility::string_no_trailing(symbol::get(token).constant.value);
    }
   }
  }
 }

 static void fold(vector<string>& output, vector<string>& stash, const string& incoming_token = "", const string& stop_at = "") {
  while (
   output.size() >= 2
   && utility::is_number(output[output.size() - 1])
   && utility::is_number(output[output.size() - 2])
   && !stash.empty()
   && (stop_at.empty() || stash.back() != stop_at)
   && (incoming_token.empty() || (metic::precedences.count(stash.back()) && metic::precedences.at(stash.back()) >= metic::precedences.at(incoming_token)))
  ) {
   fpu b = fpu(stod(output.back())); output.pop_back();
   fpu a = fpu(stod(output.back())); output.pop_back();
   octo opcode = metic::opcodes.at(stash.back()); stash.pop_back();

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
    OP(div, b ? a / b : SENTINEL)
    OP(mod, b ? a - fpu(floor(cast(double, a / b))) * b : SENTINEL)
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
     fold(output, stash, "", "(");
     if (!stash.empty() && stash.back() != "(") {
      output.push_back(stash.back());
      stash.pop_back();
     }
    }
    if (!paren_args_count.empty()) {paren_args_count.back()++;}
    continue;
   }

   // unary "-"
   if (token == "-" && (i == 0 || metic::operations.count(tokens[i-1]) || tokens[i-1] == "(" || tokens[i-1] == ",")) {
    // merge with number
    if (i + 1 < tokens.size() && utility::is_number(tokens[i + 1])) {token = "-" + tokens[++i];}
    else {token = "neg";}
   }

   // callable
   if (utility::is_identifier(token) && i + 1 < tokens.size() && tokens[i + 1] == "(") {
    stash.push_back(token);
    continue;
   }

   if (metic::operations.count(token)) {
    // fold constant
    fold(output, stash, token);

    // pop deferred operator
    while (
     !stash.empty()
     && metic::operations.count(stash.back())
     // if higher precedence
     && (metic::precedences.at(stash.back()) > metic::precedences.at(token)
      // or not foldable
      || (metic::precedences.at(stash.back()) == metic::precedences.at(token)
       && (output.empty()
        || i + 1 >= tokens.size()
        || !utility::is_number(output.back())
        || !utility::is_number(tokens[i + 1])
       )
      )
     )
    ) {
     output.push_back(stash.back());
     stash.pop_back();
    }
    stash.push_back(token);
   }

   else if (token == "(" || token == "[") {
    stash.push_back(token);
    if (token == "(" && i > 0 && utility::is_identifier(tokens[i-1])) {paren_args_count.push_back(0);}
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

    // tag callable token with the amount of provided argument
    if (token == ")" && !stash.empty()) {
     if (utility::is_identifier(stash.back())) {
      if (!paren_args_count.empty()) {
       output.push_back(stash.back() + tag::callable_args + to_string(cast(u32, paren_args_count.back())));
       paren_args_count.pop_back();
      }
      stash.pop_back();
     }

     if (!paren_args_count.empty() && paren_args_count.back() == 0) {paren_args_count.back() = 1;}
    }

    // emit offset operator after close bracket
    else if (token == "]") {
     output.push_back(tag::offset);
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

 vector<vector<string>> tokenize(const string& line) {
  vector<vector<string>> tokens = breakdown(line);
  substitute(tokens);
  for (vector<string>& token : tokens) {token = postfix(token);}
  return tokens;
 }
}