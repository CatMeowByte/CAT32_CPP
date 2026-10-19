#pragma once

#include "core/constant.hpp"

#define MAYBE_UNUSED __attribute__((unused))

constexpr u32 font_special[32] = {
 0x20, 0x2591, 0x2592, 0x2593, 0x2588, 0x2500, 0x2502, 0x250C, 0x2510, 0x2514, 0x2518, 0x251C, 0x2524, 0x252C, 0x2534, 0x253C,
 0x2022, 0xB0, 0xA9, 0xAE, 0x2122, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xFFFD
};

constexpr u8 palette_default[16] = {0x00,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F};

namespace memory {
 extern octo raw[SYSTEM::MEMORY];

 // memory tree table generator
 // only for development convenience

 #define region(region_name,region_address,region_size,...) \
  static constexpr address region_name##_address = region_address; \
  static constexpr u32 region_name##_size = region_size; \
  static constexpr address region_name##_next = region_name##_address+region_name##_size; \
  MAYBE_UNUSED static octo* region_name##_octo = reinterpret(octo*, memory::raw+region_name##_address); \
  MAYBE_UNUSED static fpu* region_name##_fpu = reinterpret(fpu*, memory::raw+region_name##_address); \
  namespace region_name { \
   __VA_ARGS__ \
  }

 #define iocto(item_name,item_address) static constexpr u32 item_name##_address = item_address; \
  static constexpr u32 item_name##_length = 1; \
  static constexpr u32 item_name##_size = sizeof(octo); \
  static constexpr address item_name##_next = item_name##_address+item_name##_size; \
  static octo& item_name = *reinterpret(octo*, memory::raw+item_name##_address);

 #define ifpu(item_name,item_address) static constexpr u32 item_name##_address = item_address; \
  static constexpr u32 item_name##_length = 1; \
  static constexpr u32 item_name##_size = sizeof(fpu); \
  static constexpr address item_name##_next = item_name##_address+item_name##_size; \
  static fpu& item_name = *reinterpret(fpu*, memory::raw+item_name##_address);

 #define bocto(item_name,item_address,item_length) static constexpr u32 item_name##_address = item_address; \
  static constexpr u32 item_name##_length = item_length; \
  static constexpr u32 item_name##_size = sizeof(octo)*item_length; \
  static constexpr address item_name##_next = item_name##_address+item_name##_size; \
  MAYBE_UNUSED static octo* item_name = reinterpret(octo*, memory::raw+item_name##_address); \
  MAYBE_UNUSED static fpu* item_name##_fpu = reinterpret(fpu*, memory::raw+item_name##_address);

 #define bfpu(item_name,item_address,item_length) static constexpr u32 item_name##_address = item_address; \
  static constexpr u32 item_name##_length = item_length; \
  static constexpr u32 item_name##_size = sizeof(fpu)*item_length; \
  static constexpr address item_name##_next = item_name##_address+item_name##_size; \
  MAYBE_UNUSED static fpu* item_name = reinterpret(fpu*, memory::raw+item_name##_address); \
  MAYBE_UNUSED static octo* item_name##_octo = reinterpret(octo*, memory::raw+item_name##_address);

 #define check(region_name,item_name) static_assert(item_name##_next <= region_name##_next, "region overflow");

 #include "core/memory_layout.hpp"

 #undef region
 #undef iocto
 #undef ifpu
 #undef bocto
 #undef bfpu
 #undef check

 // reset
 void reset();

 // unaligned
 inline u16 unaligned_16_read(octo* ptr) {return cast(u16, ptr[0]) | (cast(u16, ptr[1]) << 8);}
 inline void unaligned_16_write(octo* ptr, u16 value) {ptr[0] = cast(octo, value & 0xFF); ptr[1] = cast(octo, (value >> 8) & 0xFF);}
 inline s32 unaligned_32_read(octo* ptr) {return cast(s32, ptr[0]) | (cast(s32, ptr[1]) << 8) | (cast(s32, ptr[2]) << 16) | (cast(s32, ptr[3]) << 24);}
 inline void unaligned_32_write(octo* ptr, s32 value) {ptr[0] = cast(octo, value & 0xFF); ptr[1] = cast(octo, (value >> 8) & 0xFF); ptr[2] = cast(octo, (value >> 16) & 0xFF); ptr[3] = cast(octo, (value >> 24) & 0xFF);}
}

namespace active {
 // pointer layout
 // need to synchronize with the tree
 using namespace memory::vm::process::p0;
 struct Process_Logic {
  union {
   octo code_octo[logic::code_length];
   fpu code_fpu[logic::code_length / sizeof(fpu)];
  };
  fpu stacker;
  fpu slotter;
  fpu writer;
  fpu counter;
  fpu framer;
  fpu frames[121];
 };

 struct Process_Local {
  octo sprite[8192];
 };

 extern Process_Logic* logic;
 extern Process_Local* local;

 void index(u8 n);
}

namespace memory {
 // internal
 inline fpu pop() {return active::logic->code_fpu[active::logic->stacker++.i()];}
 inline void push(fpu value) {active::logic->code_fpu[(--active::logic->stacker).i()] = value;}
}

#undef MAYBE_UNUSED