#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"
#include "module/audio.hpp"

namespace audio {
 void tone(u8 channel, float key = SENTINEL, float duty = SENTINEL) {
  if (channel > 3) {return;}
  using namespace memory::vm::global;
  hardware_io::frequency[channel] = (key == constant::sentinel) ? 0 : 440 * pow(2, (key - 69) / 12);
  if (duty != constant::sentinel) {hardware_io::duty[channel] = duty;}
 }

 namespace wrap {
  OPCODE(tone, {
   float duty = memory::pop();
   float key = memory::pop();
   u8 channel = memory::pop();
   audio::tone(channel, key, duty);
  })
 }

 MODULE(
  module::add("audio", "tone", wrap::tone, 3, {SENTINEL, SENTINEL});
 )
}