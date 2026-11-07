#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"
#include "core/tool.hpp"
#include "module/log.hpp"

namespace log_ops {
 void out(string text) {
  using namespace memory::vm::global::log;

  u16 head_index = head.i();
  u16 neck_index = neck.i();
  u16 tail_index = tail.i();
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

  head = (head_index + SYSTEM::LOG - 1) % SYSTEM::LOG;
  neck = neck_index;
  tail = tail_index;
 }

 void output(string text) {
  using namespace memory::vm::global::log;

  out(text);

  u16 neck_index = neck.i();
  u16 head_index = head.i();

  // print to stdout
  u16 print_length = (head_index - neck_index + SYSTEM::LOG) % SYSTEM::LOG + 1;
  u16 first_length = min(print_length, cast(u16, SYSTEM::LOG - neck_index));
  cout.write(reinterpret(const char*, &data[neck_index]), first_length);
  if (first_length < print_length) {cout.write(reinterpret(const char*, &data[0]), print_length - first_length);}
  cout.flush();

  // new line
  neck = (head_index + 1) % SYSTEM::LOG;
  head = neck;
 }

 namespace wrap {
  OPCODE(out, {
   slot_logic slot_text = memory::pop().a();

   string string_text = tool::text::pick(slot_text);

   log_ops::out(string_text);
  })

  OPCODE(output, {
   slot_logic slot_text = memory::pop().a();

   string string_text = tool::text::pick(slot_text);

   log_ops::output(string_text);
  })
 }

 MODULE(
  module::add("log", "out", wrap::out, 1);
  module::add("log", "output", wrap::output, 1);
 )
}