#include "core/memory.hpp"
#include "core/opcode.hpp"
#include "core/utility.hpp"
#include "module/builtin.hpp"

namespace utility {
 bool is_number(const string& text) {
  if (text.empty()) {return false;}
  u32 i = 0;
  bool has_digit = false;
  bool has_dot = false;

  if (text[0] == '-' || text[0] == '+') i++;

  for (; i < text.length(); i++) {
   if (isdigit(text[i])) {
    has_digit = true;
    continue;
   }
   if (text[i] == '.' && !has_dot) {
    has_dot = true;
    continue;
   }
   return false;
  }
  return has_digit;
 }

 bool is_hex(const string& text) {
  if (text.size() < 3 || text[0] != '0' || (text[1] != 'x' && text[1] != 'X')) {return false;}
  bool has_dot = false;
  for (size_t i = 2; i < text.size(); i++) {
   char c = text[i];
   if (c == '.') {
    if (has_dot) {return false;}
    has_dot = true;
    continue;
   }
   bool is_digit = (c >= '0' && c <= '9');
   bool is_lower = (c >= 'a' && c <= 'f');
   bool is_upper = (c >= 'A' && c <= 'F');
   if (!(is_digit || is_lower || is_upper)) {return false;}
  }
  return true;
 }

 double hex_to_number(const string& text) {
  u64 dot = text.find('.');
  string hex = "";
  if (dot == string::npos) {
   hex += string(8 - hex.length(), '0');
   hex = text.substr(2) + hex;
  } else {
   hex += text.substr(dot + 1);
   hex += string(8 - hex.length(), '0');
   hex = text.substr(2, dot - 2) + hex;
  }
  hex = string(16 - hex.length(), '0') + hex;
  double value = stoll(hex, nullptr, 16);
  return value / double(1ULL << 32);
 }

 bool is_bin(const string& text) {
  if (text.size() < 3) {return false;}
  if (!(text[0] == '0' && (text[1] == 'b' || text[1] == 'B'))) {return false;}
  bool has_dot = false, has_digit = false;
  for (u32 i = 2; i < text.size(); i++) {
   char c = text[i];
   if (c == '.') {if (has_dot) {return false;} has_dot = true; continue;}
   if (c == '0' || c == '1') {has_digit = true; continue;}
   return false;
  }
  return has_digit;
 }

 double bin_to_number(const string& text) {
  u64 dot = text.find('.');
  string bin = "";
  if (dot == string::npos) {
   bin += string(32 - bin.length(), '0'); // fractional pad
   bin = text.substr(2) + bin;
  } else {
   bin += text.substr(dot + 1);
   bin += string(32 - bin.length(), '0');
   bin = text.substr(2, dot - 2) + bin;
  }
  bin = string(64 - bin.length(), '0') + bin;
  u64 value = bin.empty() ? 0ULL : stoull(bin, nullptr, 2);
  return value / double(1ULL << 32);
 }

 string string_no_trailing(double value) {
  string s = to_string(value);
  s.erase(s.find_last_not_of('0') + 1, string::npos); // remove trailing zeros
  if(s.back()=='.') {s.pop_back();} // remove trailing dot
  return s;
 }

 string string_pick(addr address) {
  string out;
  for (s32 i = 0; i < cast(s32, memory[address]); i++) {out += cast(char, cast(u8, memory[address + 1 + i]));}
  return out;
 }

 namespace wrap {
  addr see(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(1)
   fpu literal_value = opfunc::pop(0);
   float decimal_value = cast(float, literal_value);

   // format hex
   ostringstream hex_out;
   hex_out.setf(ios::uppercase);
   hex_out << hex;
   hex_out << setw(8);
   hex_out << setfill('0');
   hex_out << cast(s32, literal_value);
   string hex_string = hex_out.str();

   int dot_position = fpu::DECIMAL_WIDTH / 4;
   string fixed_hex = hex_string.substr(0, 8 - dot_position) + "." + hex_string.substr(8 - dot_position);

   // format float
   string decimal_string = utility::string_no_trailing(decimal_value);

   cout << "[SEE] [" << cast(s32, literal_value) << "] [" << fixed_hex << "] [" << decimal_string << "]" << endl;
   return SENTINEL;
  }

  addr wait(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(1)
   sleeper = cast(s32, opfunc::pop(0));
   return SENTINEL;
  }
 }

 void builtin_register() {
  builtin::add("see", wrap::see);
  builtin::add("wait", wrap::wait);
 }
}