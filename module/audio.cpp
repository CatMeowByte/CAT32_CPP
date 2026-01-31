#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"
#include "module/audio.hpp"

namespace audio {
 void tone(u8 channel, float key = SENTINEL, float duty = SENTINEL) {
  if (channel > 3) {return;}
  using namespace memory::vm::ram_global;
  hardware_io::frequency[channel] = (key == cast(float, constant::sentinel)) ? 0 : 440 * pow(2, (key - 69) / 12);
  if (duty != cast(float, constant::sentinel)) {hardware_io::duty[channel] = duty;}
 }

 namespace wrap {
  addr tone(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(3)
   float duty = memory::pop();
   float key = memory::pop();
   u8 channel = memory::pop();
   audio::tone(channel, key, duty);
   OPDONE;
  }
 }

 MODULE(
  module::add("audio", "tone", wrap::tone, 3, {SENTINEL, SENTINEL});
 )
}