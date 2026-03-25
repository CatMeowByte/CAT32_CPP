#include "core/constant.hpp"  // IWYU pragma: keep
#include "core/memory.hpp"  // IWYU pragma: keep
#include "core/module.hpp"
#include "core/opcode.hpp"
#include "library/sdl.hpp"  // IWYU pragma: keep
#include "module/button.hpp"

namespace button {
 using namespace memory::vm::global;
 constexpr u8 KEY_COUNT = 8;
 constexpr u8 KEY_ALT = 4;

 static const struct {fpu& memory; str keys[KEY_ALT];} keymap[KEY_COUNT] = {
  {hardware_io::up, {"up", "w", nullptr, nullptr}},
  {hardware_io::down, {"down", "s", nullptr, nullptr}},
  {hardware_io::left, {"left", "a", nullptr, nullptr}},
  {hardware_io::right, {"right", "d", nullptr, nullptr}},
  {hardware_io::primary, {"space", "return", "j", "x"}},
  {hardware_io::secondary, {"left ctrl", "right ctrl", "k", "z"}},
  {hardware_io::context, {"left shift", "q", "l", nullptr}},
  {hardware_io::trigger, {"right shift", "e", "i", nullptr}},
 };

 void update() {
  for (u8 i = 0; i < KEY_COUNT; i++) {
   bool pressed = false;
   for (u8 j = 0; j < KEY_ALT && keymap[i].keys[j]; j++) {if (sdl::is_key_pressed(keymap[i].keys[j])) {pressed = true; break;}}
   if (pressed) {keymap[i].memory++;}
   else if (keymap[i].memory > fpu(0)) {keymap[i].memory = -1;}
   else if (keymap[i].memory == fpu(-1)) {keymap[i].memory = 0;}
  }
 }

 namespace wrap {
  OPCODE(button, {
   fpu index = memory::pop();
   memory::push(keymap[cast(u8, index) % KEY_COUNT].memory);
  })
 }

 MODULE(
  module::add("", "button", wrap::button, 1);
 )
}