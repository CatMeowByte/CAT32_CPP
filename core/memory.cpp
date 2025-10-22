#include "core/memory.hpp"

namespace memory {
 octo raw[SYSTEM::MEMORY];

 void reset() {
  memset(raw, 0, SYSTEM::MEMORY);

  using namespace vm::ram_global::constant;
  zero = fpu(0.0);
  sentinel = fpu(0x80000000, true);
  pi = fpu(3.14159265358979323846);
  tau = fpu(6.28318530717958647692);
  euler = fpu(2.71828182845904523536);

  using namespace vm::process::app;
  using namespace ram_local;
  stacker = fpu(field_length);
  slotter = fpu(cast(s32, (field_address - ram_local_address) / sizeof(fpu)));
 }
}