#include "core/utility.hpp"
#include <cassert>

namespace utility {
 bool is_number(str text) {
  if (!text || !*text) {return false;}
  u32 i = 0;
  bool has_digit = false;
  bool has_dot = false;

  if (text[0] == '-' || text[0] == '+') i++;

  for (; text[i] != '\0'; i++) {
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

 string string_no_trailing(double value) {
  string s = to_string(value);
  s.erase(s.find_last_not_of('0') + 1, string::npos); // remove trailing zeros
  if(s.back()=='.') {s.pop_back();} // remove trailing dot
  return s;
 }
}