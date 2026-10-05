#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"

namespace math {
 fpu math_sin(fpu angle) {
  using namespace memory::vm::global::constant;

  s32 fractional = angle.r() & 0xFFFF;
  fpu pos = (4 * (sin_length + 1)) * fpu::raw(0x4000 - abs((fractional & 0x7FFF) - 0x4000));
  u8 i = pos.i();
  fpu result = (&zero)[i] + (pos - fpu(i)) * ((&zero)[i + 1] - (&zero)[i]);

  return (fractional & 0x8000) ? -result : result;
 }

 pair<s32, s32> math_cordic(s64 x, s64 y) {
  using namespace memory::vm::global::constant;
  constexpr s32 inv_tau = fpu(0.15915494309189533577).r();
  constexpr s32 inv_gain = fpu(0.60725293510313937961).r();

  s64 angle = 0;

  if (x < 0) {
   x = -x;
   y = -y;
   angle = (y >= 0) ? -pi.r() : pi.r();
  }

  for (s8 i = cordic_length - 1; i >= -1; i--) { // -1 to reach epsilon
   s8 shift = cordic_length - 1 - i;
   s32 step = cordic[i].r();
   s64 x_new = 0;
   if (y >= 0) {
    x_new = x + (y >> shift);
    y = y - (x >> shift);
    x = x_new;
    angle = angle + step;
   } else {
    x_new = x - (y >> shift);
    y = y + (x >> shift);
    x = x_new;
    angle = angle - step;
   }
  }

  return {cast(s32, (angle * inv_tau) >> fpu::WIDTH), cast(s32, (x * inv_gain) >> fpu::WIDTH)};
 }

 namespace wrap {
  OPCODE(sin, {
   memory::push(math_sin(memory::pop()));
  })

  OPCODE(cos, {
   memory::push(math_sin(memory::pop() + fpu(0.25)));
  })

  OPCODE(atan2, {
   s32 y = memory::pop().r();
   s32 x = memory::pop().r();

   memory::push(fpu::raw(math_cordic(x, y).first));
  })

  OPCODE(hypot, {
   s32 y = memory::pop().r();
   s32 x = memory::pop().r();

   memory::push(fpu::raw(math_cordic(x, y).second));
  })
 }

 MODULE(
  module::add("math", "sin", wrap::sin, 1);
  module::add("math", "cos", wrap::cos, 1);
  module::add("math", "atan2", wrap::atan2, 2);
  module::add("math", "hypot", wrap::hypot, 2);
 )
}
