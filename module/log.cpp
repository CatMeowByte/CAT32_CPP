#include "core/define.hpp"
#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"
#include "core/utility.hpp"
#include "module/log.hpp"

namespace log_ops {
 void out(string text) {
  using namespace memory::vm::global::log;

  u32 head_index = head.r();
  u32 neck_index = neck.r();
  u32 tail_index = tail.r();
  u32 text_length = text.size();

  // overflow
  if ((head_index - neck_index + SYSTEM::LOG) % SYSTEM::LOG + text_length + 1 >= SYSTEM::LOG) {
   if (head_index != neck_index) {head_index = (head_index + 1) % SYSTEM::LOG;} // new line
   neck_index = head_index;
   text = "log: buffer overflow";
   text_length = text.size();
  }

  bool is_ouroboros = false;
  u32 text_index = 0;
  while (text_index < text_length + 1 || is_ouroboros) {
   if (is_ouroboros) {
    if (data[tail_index] == '\n') {is_ouroboros = false;}
    tail_index = (tail_index + 1) % SYSTEM::LOG;
   }
   else if ((head_index + 1) % SYSTEM::LOG == tail_index) {is_ouroboros = true;}
   else {
    data[head_index] = (text_index < text_length) ? text[text_index] : '\n';
    head_index = (head_index + 1) % SYSTEM::LOG;
    text_index++;
   }
  }

  head = fpu::raw((head_index + SYSTEM::LOG - 1) % SYSTEM::LOG);
  neck = fpu::raw(neck_index);
  tail = fpu::raw(tail_index);
 }

 void output(string text) {
  using namespace memory::vm::global::log;

  u32 head_index = head.r();
  u32 tail_index = tail.r();
  if (head_index != tail_index) {head_index = (head_index + 1) % SYSTEM::LOG;} // new line

  head = fpu::raw(head_index);
  neck = fpu::raw(head_index);
  out(text);
 }

 namespace wrap {
  OPCODE(out, {
   address_logic address_text = memory::pop().a();
   string string_text = utility::string_pick(address_text);
   log_ops::out(string_text);
  })

  OPCODE(output, {
   address_logic address_text = memory::pop().a();
   string string_text = utility::string_pick(address_text);
   log_ops::output(string_text);
  })
 }

 MODULE(
  module::add("log", "out", wrap::out, 1);
  module::add("log", "output", wrap::output, 1);
 )
}