#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"
#include "core/tool.hpp"
#include "module/stripe.hpp"

namespace stripe_ops {
 s32 differ(const string& text_a, const string& text_b) {
  u32 min_len = (text_a.length() < text_b.length()) ? text_a.length() : text_b.length();

  for (u32 i = 0; i < min_len; i++) {
   u8 char_a = text_a[i];
   u8 char_b = text_b[i];
   if (char_a < char_b) {return -1;}
   if (char_a > char_b) {return 1;}
  }

  if (text_a.length() < text_b.length()) {return -1;}
  if (text_a.length() > text_b.length()) {return 1;}
  return 0;
 }

 s32 order(const string& text_a, const string& text_b) {
  u32 pos_a = 0;
  u32 pos_b = 0;

  while (pos_a < text_a.length() && pos_b < text_b.length()) {
   u8 char_a = text_a[pos_a];
   u8 char_b = text_b[pos_b];

   bool digit_a = (char_a >= '0' && char_a <= '9');
   bool digit_b = (char_b >= '0' && char_b <= '9');

   if (digit_a && digit_b) {
    while (pos_a < text_a.length() && text_a[pos_a] == '0') {pos_a++;}
    while (pos_b < text_b.length() && text_b[pos_b] == '0') {pos_b++;}

    s8 digit_result = 0;
    while (pos_a < text_a.length() || pos_b < text_b.length()) {
     bool digit_continue_a = pos_a < text_a.length() && text_a[pos_a] >= '0' && text_a[pos_a] <= '9';
     bool digit_continue_b = pos_b < text_b.length() && text_b[pos_b] >= '0' && text_b[pos_b] <= '9';
     if (digit_continue_a && digit_continue_b) {
      if (!digit_result && text_a[pos_a] != text_b[pos_b]) {digit_result = (text_a[pos_a] < text_b[pos_b]) ? -1 : 1;}
      pos_a++;
      pos_b++;
     }
     else if (digit_continue_a) {return 1;}
     else if (digit_continue_b) {return -1;}
     else {break;}
    }

    if (digit_result) {return digit_result;}
   }
   else {
    if (char_a < char_b) {return -1;}
    if (char_a > char_b) {return 1;}
    pos_a++;
    pos_b++;
   }
  }

  if (pos_a < text_a.length()) {return 1;}
  if (pos_b < text_b.length()) {return -1;}
  return 0;
 }

 double to_n(const string& text) {
  u32 i = 0;
  while (i < text.length() && (text[i] == ' ' || text[i] == '\t' || text[i] == '\n' || text[i] == '\r')) {i++;}

  s8 neg = 1;
  if (i < text.length() && text[i] == '-') {neg = -1; i++;}

  double result = 0;
  double div = 0;

  while (i < text.length()) {
   if (text[i] >= '0' && text[i] <= '9') {
    if (div) {result += (text[i] - '0') / div; div *= 10;}
    else {result = result * 10 + (text[i] - '0');}
   }
   else if (text[i] == '.' && !div) {div = 10;}
   else {break;}
   i++;
  }

  return result * neg;
 }

 namespace wrap {
  OPCODE(differ, {
   slot_logic slot_b = memory::pop().a();
   slot_logic slot_a = memory::pop().a();

   string text_a = tool::text::pick(slot_a);
   string text_b = tool::text::pick(slot_b);

   s8 result = stripe_ops::differ(text_a, text_b);
   memory::push(result);
  })

  OPCODE(order, {
   slot_logic slot_b = memory::pop().a();
   slot_logic slot_a = memory::pop().a();

   string text_a = tool::text::pick(slot_a);
   string text_b = tool::text::pick(slot_b);

   s8 result = stripe_ops::order(text_a, text_b);
   memory::push(result);
  })

  OPCODE(to_n, {
   slot_logic slot_text = memory::pop().a();

   string string_text = tool::text::pick(slot_text);

   double result = stripe_ops::to_n(string_text);
   memory::push(result);
  })

  OPCODE(from_n, {
   double number = memory::pop();
   slot_logic slot_destination = memory::pop().a();

   string number_text = tool::text::remove_trailing(number);
   tool::text::put(slot_destination, number_text);
   memory::push(slot_destination);
  })

  OPCODE(add, {
   slot_logic slot_b = memory::pop().a();
   slot_logic slot_a = memory::pop().a();
   slot_logic slot_destination = memory::pop().a();

   string text_a = tool::text::pick(slot_a);
   string text_b = tool::text::pick(slot_b);

   string result = text_a + text_b;
   tool::text::put(slot_destination, result);
   memory::push(slot_destination);
  })

  OPCODE(sub, {
   u32 length = memory::pop();
   u32 start = memory::pop();
   slot_logic slot_source = memory::pop().a();
   slot_logic slot_destination = memory::pop().a();

   string text = tool::text::pick(slot_source);

   string result = text.substr(start, length);
   tool::text::put(slot_destination, result);
   memory::push(slot_destination);
  })

  OPCODE(get_char, {
   u32 index = memory::pop();
   slot_logic slot_text = memory::pop().a();

   u8 result = active::logic->code_octo[slot_text * sizeof(fpu) + index];
   memory::push(fpu::raw(result));
  })

  OPCODE(len, {
   s16 length = memory::pop().i();
   slot_logic slot_text = memory::pop().a();

   u16 length_old = tool::stripe::get_len(slot_text);
   if (length != SENTINEL) {tool::stripe::set_len(slot_text, length);}
   memory::push(fpu(length_old));
  })

  OPCODE(cap, {
   slot_logic slot_text = memory::pop().a();

   u16 capacity = tool::stripe::get_cap(slot_text);
   memory::push(fpu(capacity));
  })

  OPCODE(count, {
   slot_logic slot_text = memory::pop().a();

   u16 count = tool::stripe::get_len(slot_text) / sizeof(fpu);
   memory::push(fpu(count));
  })
 }

 MODULE(
  module::add("str", "differ", wrap::differ, 2);
  module::add("str", "order", wrap::order, 2);
  module::add("str", "to_n", wrap::to_n, 1);
  module::add("str", "from_n", wrap::from_n, 2);
  module::add("str", "add", wrap::add, 3);
  module::add("str", "sub", wrap::sub, 4);
  module::add("str", "char", wrap::get_char, 2);
  module::add("str", "len", wrap::len, 2, {SENTINEL});
  module::add("str", "cap", wrap::cap, 1);
  module::add("str", "count", wrap::count, 1);
 )
}