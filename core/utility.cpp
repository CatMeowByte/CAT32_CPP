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
  size_t dot = text.find('.');
  string int_part = (dot == string::npos) ? text.substr(2) : text.substr(2, dot - 2);
  string frac_part = (dot == string::npos) ? "" : text.substr(dot + 1);
  double value = 0;
  if (!int_part.empty()) {value = stoi(int_part, nullptr, 16);}
  if (!frac_part.empty()) {
   double frac = 0;
   double base = 1;
   for (char c : frac_part) {
    base /= 16.0;
    int digit = (c >= '0' && c <= '9') ? (c - '0')
     : (c >= 'a' && c <= 'f') ? (c - 'a' + 10)
     : (c >= 'A' && c <= 'F') ? (c - 'A' + 10)
     : 0;
    frac += digit * base;
   }
   value += frac;
  }
  return value;
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