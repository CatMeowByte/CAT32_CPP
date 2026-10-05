# pragma once

#include "core/constant.hpp" // IWYU pragma: keep
#include "core/memory.hpp" // IWYU pragma: keep

namespace tool {
 constexpr u32 hash(str text, u32 seed = 2166136261) {
  return *text ? hash(text + 1, (seed ^ cast(u8, *text)) * 16777619) : seed;
 }

 namespace convert {
  double hex_to_number(const string& text);
  double bin_to_number(const string& text);
 }

 namespace text {
  bool is_number(const string& text);
  bool is_identifier(const string& text);
  bool is_hex(const string& text);
  bool is_bin(const string& text);

  string remove_trailing(double value);

  string pick(slot_logic address);
  void put(slot_logic address, const string& text);
 }

 namespace stripe {
  #define ADDRESS (active::logic->code_octo + (address - 1) * sizeof(fpu))
  inline u16 get_len(slot_logic address) {return memory::unaligned_16_read(ADDRESS);}
  inline u16 get_cap(slot_logic address) {return memory::unaligned_16_read(ADDRESS + 2);}
  inline void set_len(slot_logic address, u16 len) {memory::unaligned_16_write(ADDRESS, len);}
  inline void set_cap(slot_logic address, u16 cap) {memory::unaligned_16_write(ADDRESS + 2, cap);}
  #undef ADDRESS
 }
}
