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

 bool is_number(str text);

 string string_no_trailing(double value);
}
