#pragma once

#include "core/constant.hpp"

#define MAYBE_UNUSED __attribute__((unused))

extern const u32 font_special[32];

namespace memory {
 extern octo raw[SYSTEM::MEMORY];

 // memory tree table generator
 // only for development convenience

 #define region(region_name,region_address,region_size,...) \
  static constexpr addr region_name##_address = region_address; \
  static constexpr u32 region_name##_size = region_size; \
  static constexpr addr region_name##_next = region_name##_address+region_name##_size; \
  MAYBE_UNUSED static octo* region_name##_octo = cast(octo*, memory::raw+region_name##_address); \
  MAYBE_UNUSED static fpu* region_name##_fpu = cast(fpu*, cast(void*, memory::raw+region_name##_address)); \
  namespace region_name { \
   __VA_ARGS__ \
  }

 #define iocto(item_name,item_address) static constexpr addr item_name##_address = item_address; \
  static constexpr u32 item_name##_length = 1; \
  static constexpr u32 item_name##_size = sizeof(octo); \
  static constexpr addr item_name##_next = item_name##_address+item_name##_size; \
  static octo& item_name = *cast(octo*, memory::raw+item_name##_address);

 #define ifpu(item_name,item_address) static constexpr addr item_name##_address = item_address; \
  static constexpr u32 item_name##_length = 1; \
  static constexpr u32 item_name##_size = sizeof(fpu); \
  static constexpr addr item_name##_next = item_name##_address+item_name##_size; \
  static fpu& item_name = *cast(fpu*, cast(void*, memory::raw+item_name##_address));

 #define bocto(item_name,item_address,item_length) static constexpr addr item_name##_address = item_address; \
  static constexpr u32 item_name##_length = item_length; \
  static constexpr u32 item_name##_size = sizeof(octo)*item_length; \
  static constexpr addr item_name##_next = item_name##_address+item_name##_size; \
  MAYBE_UNUSED static octo* item_name = cast(octo*, memory::raw+item_name##_address);

 #define bfpu(item_name,item_address,item_length) static constexpr addr item_name##_address = item_address; \
  static constexpr u32 item_name##_length = item_length; \
  static constexpr u32 item_name##_size = sizeof(fpu)*item_length; \
  static constexpr addr item_name##_next = item_name##_address+item_name##_size; \
  MAYBE_UNUSED static fpu* item_name = cast(fpu*, cast(void*, memory::raw+item_name##_address));

 #define check(region_name,item_name) static_assert(item_name##_next <= region_name##_next, "region overflow");

 region(vm, 0, SYSTEM::MEMORY, // map of the entire memory
  region(ram_global, vm_address, 32768, // presistent memory
   region(constant, ram_global_address, 32, // populated at startup. should be unchanged
    ifpu(zero, constant_address) // runtime zero reference
    ifpu(one, zero_next) // runtime one reference
    ifpu(sentinel, one_next) // runtime sentinel reference. not to be confused with compile time SENTINEL
    ifpu(signature, sentinel_next) // special bytes. meaningless
    ifpu(pi, signature_next) // pi
    ifpu(tau, pi_next) // tau. why is it exist if pi*2 is possible too?
    ifpu(euler, tau_next) // euler. likely almost never be used
   )
   check(constant, constant::euler)

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
    ifpu(trigger, secondary_next) // shoulder right
    ifpu(context, trigger_next) // shoulder left
    ifpu(acceleration_x, context_next) // right
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
  )
  check(ram_global, ram_global::hardware_io::magnetometer_z)

  region(process, ram_global_next, 65536, // process region
   region(app, process_address, 65536, // app 0

    bocto(bytecode, app_address, 32768) // compiled bytecode

    region(ram_local, bytecode_next, 32768, // app memory
     ifpu(stacker, ram_local_address) // stack pointer. starts from end of local memory
     ifpu(slotter, stacker_next) // variable allocator. increment on compile
     ifpu(writer, slotter_next) // points to after last bytecode
     ifpu(counter, writer_next) // bytecode execution pointer
     ifpu(sleeper, counter_next) // wait countdown on ticks
     ifpu(framer, sleeper_next) // call adress pointer
     bfpu(frames, framer_next, 121) // call adress stack
     bocto(sprite, frames_next, 8192) // 128x128 4bpp sprite data
     bfpu(field, sprite_next, 6016) // free space. last few bytes is used as stack
    )
    check(ram_local, ram_local::field)
   )
   check(app, app::ram_local::field)
  )
  check(process, process::app::ram_local::field)
 )
 check(vm, vm::process::app::ram_local::field)

 #undef region
 #undef iocto
 #undef ifpu
 #undef bocto
 #undef bfpu
 #undef check

 // reset
 void reset();

 // internal
 inline fpu pop() {using namespace vm::process::app::ram_local; return field[stacker++];}

 // unaligned
 inline s32 unaligned_32_read(octo* ptr) {return cast(s32, ptr[0]) | (cast(s32, ptr[1]) << 8) | (cast(s32, ptr[2]) << 16) | (cast(s32, ptr[3]) << 24);}
 inline void unaligned_32_write(octo* ptr, s32 value) {ptr[0] = cast(octo, value & 0xFF); ptr[1] = cast(octo, (value >> 8) & 0xFF); ptr[2] = cast(octo, (value >> 16) & 0xFF); ptr[3] = cast(octo, (value >> 24) & 0xFF);}
}

#undef MAYBE_UNUSED