# pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace utility {
 constexpr u32 hash(str text) {
  u32 hash = 5381;
  while (*text) {
   hash = ((hash << 5) + hash) + *text;
   ++text;
  }
  return hash;
 }

 bool is_number(const string& text);

 bool is_hex(const string& text);

 double hex_to_number(const string& text);

 string string_no_trailing(double value);

 string string_pick(addr address);
}
