#include "core/constant.hpp"
#include "core/interpreter.hpp"
#include "core/memory.hpp"
#include "core/opcode.hpp"
#include "core/utility.hpp"

namespace interpreter {
 static void code_add(u8 size, s32 value) {
  for (u8 i = 0; i < size; i++) {active::logic->code_octo[active::logic->writer.a() + 1 + i] = (value >> (i * 8)) & 0xFF;}
  active::logic->writer += size;
 }

 void compile(const vector<vector<string>>& line_tokens) {
  u8 indent = stoi(line_tokens[0][0]);
  vector<vector<string>> tokens(line_tokens.begin() + 1, line_tokens.end());

  if (tokens.empty()) {return;}

  // debug print formatted token
  {
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
  }

  // flag
  enum class HeaderType : u8 {None, If, While, Else, Function};
  HeaderType header_type = HeaderType::None;

  enum class AssignType : u8 {None, Declare, Set};
  AssignType assign_type = AssignType::None;

  enum class DeclareStyle : u8 {None, Variable, Constant, Stripe};
  DeclareStyle declare_style = DeclareStyle::None;

  enum class SetStyle : u8 {None, Variable, Stripe};
  SetStyle set_style = SetStyle::None;

  bool is_expression = true;
  for (const vector<string>& token : tokens) {
   if (find(token.begin(), token.end(), "=") != token.end()) {is_expression = false; break;}
  }

  bool is_return = tokens[0][0] == "return";

  // indent
  if (indent > scope::previous::indent) {
   if (indent - scope::previous::indent > 1) {cout << "caution: too much indent" << endl;}

   for (u8 indent_level = scope::previous::indent + 1; indent_level <= indent; indent_level++) {
    cout << ">>>>" << endl;

    if (scope::previous::type != scope::Type::Function) { // if, while, and else
     const octo last_opcode = active::logic->code_octo[active::logic->writer.a() - 2];

     if (last_opcode != op::jump && last_opcode != op::jumz && last_opcode != op::junz) {
      cout << "caution: last opcode before indent is not jump/jumz/junz" << endl;
     }
    }

    scope::Frame frame;
    frame.type = scope::previous::type;
    frame.line = scope::previous::line;
    frame.skip_operand = scope::previous::skip_operand;
    frame.symbol_boundary = symbol::table.size();
    scope::stack.push_back(frame);

    scope::previous::type = scope::Type::Generic;
    cout << "indent stack added with jump operand at " << scope::previous::skip_operand << endl;
   }
  }

  // dedent
  if (indent < scope::previous::indent) {
   for (u8 indent_level = scope::previous::indent; indent_level > indent; indent_level--) {
    cout << "<<<<" << endl;

    // only handle else because at "else" its a single line dedent with only one keyword
    // so this handled earlier before patching the bytecode jump operand with writer
    if (tokens[0][0] == "else") {
     header_type = HeaderType::Else;

     code_add(1, op::jump);
     code_add(2, FARLAND);
     scope::previous::skip_operand = active::logic->writer.a() - 1;
     scope::previous::type = scope::Type::Else;
    }

    const scope::Frame& frame = scope::stack.back();

    symbol::table.resize(frame.symbol_boundary);

    switch (frame.type) {
     case scope::Type::While: {
      code_add(1, op::jump);
      code_add(2, frame.line + 1); // next opcode

      // patch break
      for (address_logic patch_addr : frame.break_operands) {
       memory::unaligned_16_write(active::logic->code_octo + patch_addr, active::logic->writer.a() + 1);
       cout << "patching break at " << patch_addr << " to " << active::logic->writer.a() + 1 << endl;
      }
      break;
     }
     case scope::Type::Function: {
      if (active::logic->code_octo[active::logic->writer.a() - 1] != op::subret) { // last opcode
       cout << "implicit return emitted at " << active::logic->writer.a() << endl;
       code_add(1, op::subret);
      }
      break;
     }
     default: {break;}
    }

    memory::unaligned_16_write(active::logic->code_octo + frame.skip_operand, active::logic->writer.a() + 1);
    cout << "patching jump operand at " << frame.skip_operand << " to address " << active::logic->writer.a() + 1 << endl;

    scope::stack.pop_back();
   }
  }
  scope::previous::indent = indent;

  // set to current line after used on indent
  scope::previous::line = active::logic->writer.a() + 1;

  // set header
  {
   if (tokens[0][0] == "if") {header_type = HeaderType::If;}
   if (tokens[0][0] == "while") {header_type = HeaderType::While;}
   if (tokens[0][0] == "func") {header_type = HeaderType::Function;}
  }

  // else
  // already handled earlier in dedent
  if (header_type == HeaderType::Else) {return;}

  // break and continue
  if (tokens[0][0] == "break" || tokens[0][0] == "continue") {
   for (s32 i = scope::stack.size() - 1; i >= 0; i--) {
    if (scope::stack[i].type != scope::Type::While) {continue;}

    if (tokens[0][0] == "break") {
     code_add(1, op::jump);
     code_add(2, FARLAND);
     address_logic patch_address = active::logic->writer.a() - 1;
     scope::stack[i].break_operands.push_back(patch_address);
     cout << "break stores unpatched jump at " << patch_address << endl;
    }
    else { // continue
     code_add(1, op::jump);
     code_add(2, scope::stack[i].line);
     cout << "continue jumps to while header at " << scope::stack[i].line << endl;
    }

    return;
   }
   cout << "error: " << tokens[0][0] << " outside of while loop" << endl;
   return;
  }

  // use spacename
  if (tokens[0][0] == "use") {
   if (tokens.size() < 2) {cout << "error: use requires at least one spacename" << endl; return;}

   for (u32 i = 1; i < tokens.size(); i++) {
    if (tokens[i].size() != 1) {cout << "error: invalid spacename" << endl; return;}

    string space_name = tokens[i][0];
    u16 space_hash = utility::hash(space_name.c_str()) & 0xFFFF;
    scope::stack.back().space.push_back(space_hash);
    cout << "imported spacename \"" << space_name << "\" with hash " << space_hash << endl;
   }
   return;
  }

  // set type
  // declare
  {
   if (tokens.size() == 4 && tokens[2][0] == "=") {
    if (tokens[0][0] == "var") {
     assign_type = AssignType::Declare;
     declare_style = DeclareStyle::Variable;
    }
    if (tokens[0][0] == "con") {
     assign_type = AssignType::Declare;
     declare_style = DeclareStyle::Constant;
    }
   }
   if (
    tokens[0][0] == "str"
    && (
     (tokens.size() == 2 && tokens[1].back() == tag::offset)
     || (tokens.size() == 4 && tokens[2][0] == "=")
    )
   ) {
    assign_type = AssignType::Declare;
    declare_style = DeclareStyle::Stripe;
   }
  }

  // assign
  if (tokens.size() == 3 && tokens[1][0] == "=") {
   assign_type = AssignType::Set;
   set_style = SetStyle::Variable;

   if (tokens[0].size() == 1 && symbol::exist(tokens[0][0])) {
    const symbol::Type type = symbol::get(tokens[0][0]).type;

    // constant
    if (type == symbol::Type::Constant) {
     cout << "error: cannot assign to constant \"" << tokens[0][0] << "\"" << endl;
     return;
    }

    // stripe full
    if (type == symbol::Type::Stripe) {set_style = SetStyle::Stripe;}
   }

   // stripe indexed
   if (
    tokens[0].size() >= 3
    && tokens[0].back() == tag::offset
    && symbol::exist(tokens[0][0])
    && symbol::get(tokens[0][0]).type == symbol::Type::Stripe
   ) {
    set_style = SetStyle::Stripe;
   }
  }

  // prevent garbage stack accumulation
  code_add(1, op::prime);

  // token processing
  // for (u32 i = 0; i < tokens.size(); i++) {
  //  const vector<string>& expression = tokens[i];
  //  if (expression.size() == 1 && expression[0] == "=") {is_expression = true; continue;}

  //  // skip constant declaration value
  //  if (i == 3 && declare_style == DeclareStyle::Constant) {continue;}

  //  // skip stripe declaration identifier
  //  if (i == 1 && declare_style == DeclareStyle::Stripe) {continue;}

  //  // check recursive
  //  s32 recursive_index = -1;
  //  for (const string& token : expression) {
  //   u64 callable_tag_pos = token.find(tag::callable_args);
  //   if (!callable_tag_pos || callable_tag_pos == string::npos) {continue;}

  //   string name = token.substr(0, callable_tag_pos);
  //   if (!symbol::exist(name) || symbol::get(name).type != symbol::Type::Function) {break;}

  //   // find current function scope
  //   for (s32 k = scope::stack.size() - 1; k >= 0; k--) {
  //    if (scope::stack[k].type != scope::Type::Function) {continue;}
  //    if (scope::stack[k].header_start + 5 != symbol::get(name).address) {break;}

  //    recursive_index = k;
  //    // stash recursive argument
  //    for (u32 m = scope::stack[k].symbol_start - symbol::get(name).args_count; m < scope::stack[k].symbol_start; m++) {
  //     bytecode_append(op::takefrom, symbol::table[m].address);
  //    }
  //    break;
  //   }
  //   break;
  //  }

  //  for (u32 j = 0; j < expression.size(); j++) {
  //   const string& token = expression[j];
  //   const u64 callable_tag_pos = token.find(tag::callable_args);

  //   if (false) {}

  //   // function
  //   // run once early in specific condition to process the whole function declaration header and skip procesing the rest of token
  //   else if (header_type == HeaderType::Function && i == 1 && j == 0) {
  //    scope::last_jump_operand = cast(addr, writer) + 1;
  //    bytecode_append(op::jump, memory::vm::ram_global::constant::sentinel);
  //    scope::last_line_scope_set = scope::Type::Function;

  //    string name_tagged = expression.back();
  //    u64 declare_tag_pos = name_tagged.find(tag::callable_args);
  //    string name = name_tagged.substr(0, declare_tag_pos);
  //    addr address = cast(addr, writer);

  //    // event loop
  //    if (name == "init") {memory::unaligned_32_write(bytecode + 1, fpu(address).r()); cout << "event loop init is written at bytecode [" << address << "]" << endl;}
  //    else if (name == "step") {memory::unaligned_32_write(bytecode + 6, fpu(address).r()); cout << "event loop step is written at bytecode [" << address << "]" << endl;}
  //    else if (name == "draw") {memory::unaligned_32_write(bytecode + 11, fpu(address).r()); cout << "event loop draw is written at bytecode [" << address << "]" << endl;}

  //    // symbol
  //    symbol::Data function_symbol;
  //    function_symbol.name = name;
  //    function_symbol.address = address;
  //    function_symbol.type = symbol::Type::Function;
  //    function_symbol.args_count = 0;
  //    symbol::table.push_back(function_symbol);
  //    cout << name << " function is written at bytecode [" << address << "]" << endl;
  //    const u32 function_symbol_index = symbol::table.size() - 1; // prevent invalidation

  //    // arguments
  //    address = cast(addr, slotter);
  //    const u8 args_count = declare_tag_pos == string::npos ? 0 : stoi(name_tagged.substr(declare_tag_pos + strlen(tag::callable_args)));
  //    s32 slot = args_count - 1;
  //    string last_required = "";

  //    for (s32 i = cast(s32, expression.size()) - 2; i >= 0; i--) {
  //     const string& token = expression[i];

  //     // skip constant, must preceded by separator
  //     if (utility::is_number(token)) {
  //      if (i - 1 < 0 || expression[i - 1] != ARGUMENT_OPTIONAL) {
  //       cout << "error: constant without assignment in argument list" << endl;
  //       return;
  //      }
  //      continue;
  //     }

  //     // skip separator, must preceded by name
  //     if (token == ARGUMENT_OPTIONAL) {
  //      if (i - 1 < 0 || utility::is_number(expression[i - 1]) || expression[i - 1] == ARGUMENT_OPTIONAL || math_opcodes.count(expression[i - 1])) {
  //       cout << "error: assignment without name" << endl;
  //       return;
  //      }
  //      continue;
  //     }

  //     // reject operator
  //     if (math_opcodes.count(token)) {cout << "error: invalid expression" << endl; return;}

  //     // name
  //     if (i + 1 < cast(s32, expression.size()) && expression[i + 1] == ARGUMENT_OPTIONAL) {
  //      // optional
  //      if (i + 2 >= cast(s32, expression.size())) {cout << "error: assignment without default value" << endl; return;}
  //      if (!utility::is_number(expression[i + 2])) {cout << "error: default value must be constant expression" << endl; return;}
  //      if (!last_required.empty()) {cout << "error: optional " << token << " after required " << last_required << " is not allowed" << endl; return;}

  //      fpu default_value = stod(expression[i + 2]);
  //      symbol::table[function_symbol_index].args_default.insert(symbol::table[function_symbol_index].args_default.begin(), default_value);
  //      symbol::table[function_symbol_index].args_count++;
  //     }
  //     else {
  //      // required
  //      last_required = token;
  //      symbol::table[function_symbol_index].args_count++;
  //     }

  //     const addr args_slot = address + slot;
  //     symbol::Data arg_symbol;
  //     arg_symbol.name = token;
  //     arg_symbol.address = args_slot;
  //     arg_symbol.type = symbol::Type::Number;
  //     symbol::table.push_back(arg_symbol);
  //     bytecode_append(op::storeto, args_slot);
  //     cout << token << " is stored in " << args_slot << endl;
  //     slot--;
  //    }

  //    slotter = address + args_count;
  //    break; // ensure no further token in line is processed
  //   }

  //   // tagged callable
  //   else if (callable_tag_pos && callable_tag_pos != string::npos) { // ensure the tag is not prefix
  //    string name = token.substr(0, callable_tag_pos);
  //    u8 args_provided = stoi(token.substr(callable_tag_pos + strlen(tag::callable_args)));

  //    string space_name = "";
  //    string function_name = name;
  //    u64 dot_pos = name.find('.');

  //    if (dot_pos != string::npos) {
  //     space_name = name.substr(0, dot_pos);
  //     function_name = name.substr(dot_pos + 1);
  //    }

  //    u16 function_name_hash = utility::hash(function_name.c_str()) & 0xFFFF;
  //    u32 module_hash = 0;

  //    // unqualified
  //    if (dot_pos == string::npos) {
  //     for (s32 i = scope::stack.size() - 1; i >= 0 && !module_hash; i--) {
  //      for (u32 j = 0; j < scope::stack[i].space.size() && !module_hash; j++) {
  //       module_hash = (scope::stack[i].space[j] << 16) | function_name_hash;
  //       if (!module::exist(module_hash)) {module_hash = 0;}
  //      }
  //     }
  //    }

  //    // qualified or fallback
  //    if (!module_hash) {
  //     module_hash = ((utility::hash(space_name.c_str()) & 0xFFFF) << 16) | function_name_hash;
  //     if (!module::exist(module_hash)) {module_hash = 0;}
  //    }

  //    u8 args_total;
  //    const vector<fpu>* args_default;
  //    octo emit_opcode = op::nop;
  //    fpu emit_operand;

  //    // resolve priority
  //    // function first
  //    if (symbol::exist(name) && symbol::get(name).type == symbol::Type::Function) {
  //     const symbol::Data& function_symbol = symbol::get(name);
  //     args_total = function_symbol.args_count;
  //     args_default = &function_symbol.args_default;
  //     emit_opcode = op::subgo;
  //     emit_operand = function_symbol.address;
  //    }
  //    // module second
  //    else if (module_hash) {
  //     const module::Module& registered_module = module::table[module_hash];
  //     args_total = registered_module.args_count;
  //     args_default = &registered_module.args_default;
  //     emit_opcode = op::call;
  //     emit_operand = fpu::raw(module_hash);
  //    }
  //    // opcode last
  //    else if (opcode::exist(name)) {
  //     args_total = opcode::args_count(name);
  //     static const vector<fpu> opcode_no_defaults = {};
  //     args_default = &opcode_no_defaults;
  //     emit_opcode = opcode::get(name);
  //     emit_operand = memory::vm::ram_global::constant::signature;
  //    }

  //    if (emit_opcode == op::nop) {cout << "error: function \"" << name << "\" not found" << endl; return;}
  //    if (args_provided < args_total - args_default->size()) {cout << "error: too few arguments for " << name << " (expected at least " << cast(u32, args_total - args_default->size()) << ", got " << cast(u32, args_provided) << ")" << endl; return;}
  //    if (args_provided > args_total) {cout << "error: too many arguments for " << name << " (expected at most " << cast(u32, args_total) << ", got " << cast(u32, args_provided) << ")" << endl; return;}

  //    for (u8 i = args_provided; i < args_total; i++) {bytecode_append(op::push, (*args_default)[i - (args_total - args_default->size())]);}

  //    bytecode_append(emit_opcode, emit_operand);

  //    // pop recursive argument
  //    if (recursive_index != -1) {
  //     for (s32 j = scope::stack[recursive_index].symbol_start - 1; j >= cast(s32, scope::stack[recursive_index].symbol_start - args_total); j--) {
  //      bytecode_append(op::storeto, symbol::table[j].address);
  //     }
  //    }
  //   }

  //   // string
  //   else if (token.front() == '"' && token.back() == '"') {
  //    // skip declare
  //    if (declare_style == DeclareStyle::Stripe && i == 3 && tokens[3].size() == 1) {continue;}

  //    string content = token.substr(1, token.size() - 2); // strip quotes
  //    string name = SYMBOL_STRING_PREFIX + content;
  //    if (symbol::exist(name)) {cout << "string already exist in " << symbol::get(name).address << " is written \"" << content << "\"" << endl;}
  //    else {
  //     addr address = slotter;
  //     vector<fpu> pascal_data = utility::string_to_pascal(content);
  //     addr slot_after = cast(addr, slotter) + pascal_data.size();
  //     if (set_style == SetStyle::Stripe && tokens[0].back() != tag::offset) {
  //      address = symbol::get(tokens[0][0]).address;
  //      slot_after = slotter;
  //     }

  //     for (u32 i = 0; i < pascal_data.size(); i++) {bytecode_append(op::push, pascal_data[i]);}
  //     for (s32 i = pascal_data.size() - 1; i >= 0; i--) {bytecode_append(op::storeto, address + i);}

  //     // hidden declare
  //     if (slot_after != cast(addr, slotter)) {
  //      symbol::Data string_symbol;
  //      string_symbol.name = name;
  //      string_symbol.address = slotter;
  //      string_symbol.type = symbol::Type::Stripe;
  //      symbol::table.push_back(string_symbol);
  //      cout << "string hidden declare in " << symbol::get(name).address << " is written \"" << content << "\"" << endl;

  //      slotter = slot_after;
  //     }
  //    }

  //    if (
  //     (declare_style == DeclareStyle::Stripe && tokens.size() == 4 && tokens[3].size() == 1)
  //     || (set_style == SetStyle::Stripe && tokens[0].back() != tag::offset)
  //    ) {continue;}

  //    bytecode_append(op::push, symbol::get(name).address);
  //   }

  //   // number
  //   else if (utility::is_number(token)) {
  //    // preserve fractional
  //    double decimal = stod(token);
  //    // scale to fixed point unit
  //    double scaled = decimal * (1 << fpu::WIDTH);
  //    // round instead of truncate to zero
  //    double rounded = round(scaled);
  //    // pick the lower 32 bit
  //    u32 bits = cast(s64, rounded) & 0xFFFFFFFF;
  //    // reinterpret as s32
  //    s32 raw;
  //    memcpy(&raw, &bits, 4);
  //    bytecode_append(op::push, fpu::raw(raw));
  //   }

  //   // addressof
  //   else if (token[0] == ADDRESS_OF[0]) {
  //    string symbol_name = token.substr(1);
  //    if (!symbol::exist(symbol_name)) {
  //     cout << "error: symbol \"" << symbol_name << "\" not found" << endl;
  //     continue;
  //    }
  //    bytecode_append(op::push, symbol::get(symbol_name).address);
  //   }

  //   // symbol
  //   else if (symbol::exist(token)) {
  //    const symbol::Data& symbol = symbol::get(token);
  //    switch (symbol.type) {
  //     case symbol::Type::Number: {
  //      if (assign_type == AssignType::Set && set_style == SetStyle::Variable && !is_expression && tokens[0].size() == 1 && token == tokens[0][0]) {break;}
  //      bytecode_append(op::takefrom, symbol.address);
  //      break;
  //     }
  //     case symbol::Type::Stripe: {
  //      if (assign_type == AssignType::Set && set_style == SetStyle::Stripe && !is_expression && tokens[0].size() == 1 && token == tokens[0][0]) {break;}
  //      bytecode_append(op::push, symbol.address);
  //      break;
  //     }
  //     case symbol::Type::Function: {break;} // handled by tagged callable
  //     default: {break;}
  //    }
  //   }

  //   // stripe offset
  //   else if (token == tag::offset || token == tag::offset_at) {
  //    bytecode_append(op::add, memory::vm::ram_global::constant::signature);
  //    if (assign_type == AssignType::Set && set_style == SetStyle::Stripe && !is_expression && j == expression.size() - 1) {continue;} // stripe assignment handles storage internally
  //   if (token == tag::offset_at) {continue;} // address semantics require raw address
  //    bytecode_append(op::get, memory::vm::ram_global::constant::signature);
  //   }

  //   // math operations
  //   else if (math_opcodes.count(token)) {
  //    bytecode_append(math_opcodes.at(token), memory::vm::ram_global::constant::signature);
  //   }

  //   // invalid
  //   // terrible code, need proper non-keyword check
  //   // unfortunately, it seems like this is important and had to be this way
  //   // please reconsider
  //   else if (true
  //    && !keywords_declaration.count(token)
  //    && !keywords_control.count(token)
  //    && !(assign_type == AssignType::Declare && i == 1)
  //    && !(assign_type == AssignType::Declare && i == 2 && declare_style == DeclareStyle::Stripe)
  //    && !(assign_type == AssignType::Set && i == 0 && j == 0)
  //   )
  //   {
  //    cout << "error: invalid identifier \"" << token << "\"" << endl;
  //   }
  //  }
  // }

  // return
  if (is_return) {code_add(1, op::subret);}

  // if
  if (header_type == HeaderType::If) {
   code_add(1, op::jumz);
   code_add(2, FARLAND);
   scope::previous::skip_operand = active::logic->writer.a() - 1;
   scope::previous::type = scope::Type::If;
  }

  // while
  if (header_type == HeaderType::While) {
   code_add(1, op::jumz);
   code_add(2, FARLAND);
   scope::previous::skip_operand = active::logic->writer.a() - 1;
   scope::previous::type = scope::Type::While;
  }

  // declaration closure
  if (assign_type == AssignType::Declare) {
   string name = tokens[1][0];
   slot_logic slot = 0;

   switch (declare_style) {
    case DeclareStyle::Variable: {
     slot = (--active::logic->slotter).i();
     symbol::Data variable_symbol;
     variable_symbol.type = symbol::Type::Number;
     variable_symbol.name = name;
     variable_symbol.variable.slot = slot;
     symbol::table.push_back(variable_symbol);
     code_add(1, op::storeto);
     code_add(2, slot);
     cout << name << " is stored in " << slot << endl;
     break;
    }

    case DeclareStyle::Constant: {
     if (tokens[3].size() != 1 || !utility::is_number(tokens[3][0])) {
      cout << "error: value must be constant expression" << endl;
      break;
     }
     fpu value = stod(tokens[3][0]);
     symbol::Data constant_symbol;
     constant_symbol.type = symbol::Type::Constant;
     constant_symbol.name = name;
     constant_symbol.constant.value = value;
     symbol::table.push_back(constant_symbol);
     cout << name << " constant is " << utility::string_no_trailing(value) << endl;
     break;
    }

    case DeclareStyle::Stripe: {
     u16 size = 0;

     bool is_single_text = tokens.size() == 4
      && tokens[3].size() == 1
      && tokens[3][0].size() >= 2
      && tokens[3][0].front() == '"'
      && tokens[3][0].back() == '"';

     // explicit size
     if (tokens[1].back() == tag::offset && tokens[1].size() > 2) { // has offset value
      if (tokens[1].size() == 3 && utility::is_number(tokens[1][1])) { // offset is constant
       size = cast(u16, stod(tokens[1][1]));
      }
      else {
       cout << "error: str size must be constant expression" << endl;
       break;
      }
     }

     // implicit from content
     if (tokens.size() == 4) {
      u16 size_value = tokens[3].size();
      if (is_single_text) {size_value = (tokens[3][0].size() - 2 + 3) / 4 + 1;} // packed pascal count
      size = max(size, size_value);
     }

     if (size == 0) {
      cout << "error: str declaration requires size or initial value" << endl;
      break;
     }

     // jump to base
     active::logic->slotter -= size;
     slot = active::logic->slotter.i();

     symbol::Data stripe_symbol;
     stripe_symbol.type = symbol::Type::Stripe;
     stripe_symbol.name = name;
     stripe_symbol.variable.slot = slot;
     symbol::table.push_back(stripe_symbol);

     // fill
     if (tokens.size() == 4) {
      if (is_single_text) {
       vector<fpu> pascal_data = utility::string_to_pascal(tokens[3][0].substr(1, tokens[3][0].size() - 2));
       for (u32 i = 0; i < pascal_data.size(); i++) {
        code_add(1, op::push);
        code_add(4, pascal_data[i].r());
       }
       for (s32 i = pascal_data.size() - 1; i >= 0; i--) {
        code_add(1, op::storeto);
        code_add(2, slot + i);
       }
      }
      else {
       for (s32 i = tokens[3].size() - 1; i >= 0; i--) {
        code_add(1, op::storeto);
        code_add(2, slot + i);
       }
      }
     }

     // store size at index -1
     --active::logic->slotter;
     code_add(1, op::push);
     code_add(4, fpu(size).r());
     code_add(1, op::storeto);
     code_add(2, active::logic->slotter.i());

     cout << name << " stripe is stored in " << slot << " with size " << size << endl;
     break;
    }

    default: {break;}
   }
  }

  // assignment closure
  if (assign_type == AssignType::Set) {
   switch (set_style) {
    case SetStyle::Variable: {
     string name = tokens[0][0];

     if (!symbol::exist(name)) {cout << "error: variable \"" << name << "\" not found" << endl; return;}

     slot_logic slot = symbol::get(name).variable.slot;

     code_add(1, op::storeto);
     code_add(2, slot);
     break;
    }

    case SetStyle::Stripe: {
     // stripe full
     if (
      tokens[0].size() == 1
      && !( // not a string, already handled by hidden declare
       tokens[2].size() == 1
       && tokens[2][0].size() >= 2
       && tokens[2][0].front() == '"'
       && tokens[2][0].back() == '"'
      )
     ) {
      string name = tokens[0][0];
      slot_logic slot = symbol::get(name).variable.slot;

      for (s32 i = tokens[2].size() - 1; i >= 0; i--) {
       code_add(1, op::storeto);
       code_add(2, slot + i);
      }
     }

     // stripe indexed
     else if (tokens[0].size() >= 3) {
      code_add(1, op::set);
     }
     break;
    }

    default: {break;}
   }
  }

  return;
 }
}