#include "core/memory.hpp"
#include "core/utility.hpp"

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

 string string_no_trailing(double value) {
  string s = to_string(value);
  s.erase(s.find_last_not_of('0') + 1, string::npos); // remove trailing zeros
  if(s.back()=='.') {s.pop_back();} // remove trailing dot
  return s;
 }

 string string_pick(addr address) {
 string out;
 for (s32 i = 0; i < fpu::unpack(memory[address]); i++) {out += cast(char, fpu::unpack(memory[address + 1 + i]));}
 return out;
}
}