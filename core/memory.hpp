#pragma once

#include "core/constant.hpp"
#include "core/define.hpp"

#define MAYBE_UNUSED __attribute__((unused))

extern const u32 font_special[32];

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

 region(vm, 0, SYSTEM::MEMORY, // map of the entire memory
  region(global, vm_address, 32768, // presistent memory
   region(constant, global_address, 32, // populated at startup. should be unchanged
    ifpu(zero, constant_address) // runtime zero reference
    ifpu(one, zero_next) // runtime one reference
    ifpu(sentinel, one_next) // runtime sentinel reference. not to be confused with compile time SENTINEL
    ifpu(pi, sentinel_next) // pi
   )
   check(constant, constant::pi)

   bocto(framebuffer, constant_next, 9600) // 120*160 4bpp pixel data

   bocto(palette, framebuffer_next, 16) // 16 color remap to hardware color index. bit 7 sets visibility, 0-3 sets target index

   bocto(font, palette_next, 512) // 128 characters 16*8 grid 4x8 1bpp. 0-31 map to special characters, 32-127 standard ascii

   region(hardware_io, font_next, 128, // essential hardware device input and output
    ifpu(up, hardware_io_address) // d pad up
    ifpu(down, up_next) // d pad down
    ifpu(left, down_next) // d pad left
    ifpu(right, left_next) // d pad right
    ifpu(primary, right_next) // dedicated button. ok / a / accept
    ifpu(secondary, primary_next) // rocker press. back / b / cancel
    ifpu(context, secondary_next) // shoulder left
    ifpu(trigger, context_next) // shoulder right
    ifpu(acceleration_x, trigger_next) // right
    ifpu(acceleration_y, acceleration_x_next) // up
    ifpu(acceleration_z, acceleration_y_next) // front
    ifpu(gyroscope_x, acceleration_z_next) // roll
    ifpu(gyroscope_y, gyroscope_x_next) // pitch
    ifpu(gyroscope_z, gyroscope_y_next) // yaw
    ifpu(magnetometer_x, gyroscope_z_next) // right
    ifpu(magnetometer_y, magnetometer_x_next) // up
    ifpu(magnetometer_z, magnetometer_y_next) // front
    bfpu(frequency, magnetometer_z_next, 4) // 4 channel frequencies in hertz
    bfpu(duty, frequency_next, 4) // 4 channel duties from 0.0 to 1.0
   )
   check(hardware_io, hardware_io::duty)
   ifpu(process_index, hardware_io_next) // current active process index
  )
  check(global, global::process_index)

  region(process, global_next, 98304 * SYSTEM::PROCESS, // process region
   region(p0, process_address, 98304, // process 0

    region(logic, p0_address, 65536, // bytecode and execution memory. defined in backward order for code block dynamic size
     bfpu(frames, logic_next - sizeof(fpu) * 121, 121) // call address stack
     ifpu(framer, frames_address - sizeof(fpu)) // call address pointer
     ifpu(counter, framer_address - sizeof(fpu)) // bytecode execution pointer
     ifpu(writer, counter_address - sizeof(fpu)) // points to after last bytecode
     ifpu(slotter, writer_address - sizeof(fpu)) // variable allocator. starts from end of logic memory. grow backward. only increment on compile
     ifpu(stacker, slotter_address - sizeof(fpu)) // stack pointer. starts from end of allocator memory. grow backward
     bocto(code, logic_address, stacker_address - logic_address) // bytecode
    )
    check(logic, logic::code)

    region(local, logic_next, 32768, // data memory
     bocto(sprite, local_address, 8192) // 128x128 4bpp sprite data
    )
    check(local, local::sprite)
   )
   check(p0, p0::local::sprite)
  )
  check(process, process::p0::local::sprite)
 )
 check(vm, vm::process::p0::local::sprite)

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