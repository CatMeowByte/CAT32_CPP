# pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace utility {
 // constexpr u32 hash(str text) {
 //  return *text ? ((hash(text + 1) * 33) + static_cast<u8>(*text)) : 5381;
 // }

 bool is_number(const string& text);

 bool is_hex(const string& text);

 double hex_to_number(const string& text);

 bool is_bin(const string& text);

 double bin_to_number(const string& text);

 string string_no_trailing(double value);

 string string_pick(addr address);

 void module_register();
}
