#pragma once

#include "core/constant.hpp"

#define MAYBE_UNUSED __attribute__((unused))

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

 region(vm, 0, SYSTEM::MEMORY,
  region(ram_global, vm_address, 32768,
   region(constant, ram_global_address, 32,
    ifpu(zero, constant_address)
    ifpu(one, zero_next)
    ifpu(sentinel, one_next)
    ifpu(signature, sentinel_next)
    ifpu(pi, signature_next)
    ifpu(tau, pi_next)
    ifpu(euler, tau_next)
   )
   check(constant, constant::euler)

   bocto(framebuffer, constant_next, 9600)

   region(hardware_io, framebuffer_next, 128,
    ifpu(up, hardware_io_address)
    ifpu(down, up_next)
    ifpu(left, down_next)
    ifpu(right, left_next)
    ifpu(ok, right_next)
    ifpu(cancel, ok_next)
    ifpu(shoulder_left, cancel_next)
    ifpu(shoulder_right, shoulder_left_next)
    ifpu(acceleration_x, shoulder_right_next)
    ifpu(acceleration_y, acceleration_x_next)
    ifpu(acceleration_z, acceleration_y_next)
    ifpu(gyroscope_x, acceleration_z_next)
    ifpu(gyroscope_y, gyroscope_x_next)
    ifpu(gyroscope_z, gyroscope_y_next)
    ifpu(magnetometer_x, gyroscope_z_next)
    ifpu(magnetometer_y, magnetometer_x_next)
    ifpu(magnetometer_z, magnetometer_y_next)
   )
   check(hardware_io, hardware_io::magnetometer_z)
  )
  check(ram_global, ram_global::hardware_io::magnetometer_z)

  region(process, ram_global_next, 65536,
   region(app, process_address, 65536,

    bocto(bytecode, app_address, 32768)

    region(ram_local, bytecode_next, 32768,
     ifpu(stacker, ram_local_address)
     ifpu(slotter, stacker_next)
     ifpu(writer, slotter_next)
     ifpu(counter, writer_next)
     ifpu(sleeper, counter_next)
     ifpu(framer, sleeper_next)
     bfpu(frames, framer_next, 121)
     bocto(sprite, frames_next, 8192)
     bfpu(field, sprite_next, 6016)
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