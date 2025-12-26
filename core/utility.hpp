# pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace utility {
 constexpr u32 hash(str text, u32 seed = 2166136261) {
  return *text ? hash(text + 1, (seed ^ cast(u8, *text)) * 16777619) : seed;
 }

 bool is_number(const string& text);

 bool is_identifier(const string& text);

 bool is_hex(const string& text);

 double hex_to_number(const string& text);

 bool is_bin(const string& text);

 double bin_to_number(const string& text);

 string string_no_trailing(double value);

 string string_pick(addr address);
}
