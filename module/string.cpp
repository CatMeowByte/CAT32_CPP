#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"
#include "core/utility.hpp"
#include "module/string.hpp"

namespace string_ops {
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
    s32 num_a = 0;
    s32 num_b = 0;

    while (pos_a < text_a.length()) {
     u8 c = text_a[pos_a];
     if (c < '0' || c > '9') {break;}
     num_a = num_a * 10 + (c - '0');
     pos_a++;
    }

    while (pos_b < text_b.length()) {
     u8 c = text_b[pos_b];
     if (c < '0' || c > '9') {break;}
     num_b = num_b * 10 + (c - '0');
     pos_b++;
    }

    if (num_a < num_b) {return -1;}
    if (num_a > num_b) {return 1;}
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
  using namespace memory::vm::process::app;

  addr differ(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(2)
   addr address_b = memory::pop();
   addr address_a = memory::pop();
   string text_a = utility::string_pick(address_a);
   string text_b = utility::string_pick(address_b);
   opfunc::push(string_ops::differ(text_a, text_b));
   OPDONE;
  }

  addr order(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(2)
   addr address_b = memory::pop();
   addr address_a = memory::pop();
   string text_a = utility::string_pick(address_a);
   string text_b = utility::string_pick(address_b);
   opfunc::push(string_ops::order(text_a, text_b));
   OPDONE;
  }

  addr to_n(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(1)
   addr address = memory::pop();
   string string_text = utility::string_pick(address);
   double result = string_ops::to_n(string_text);
   opfunc::push(result);
   OPDONE;
  }

  addr from_n(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(2)
   double number = memory::pop();
   addr destination = memory::pop();
   string number_text = utility::string_no_trailing(number);
   vector<fpu> packed_pascal = utility::string_to_pascal(number_text);
   s32 buffer_size = ram_local_fpu[destination - 1];
   u32 limit = min(cast(u32, packed_pascal.size()), cast(u32, buffer_size));
   for (u32 i = 0; i < limit; i++) {ram_local_fpu[destination + i] = packed_pascal[i];}
   opfunc::push(destination);
   OPDONE;
  }

  addr add(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(3)
   addr address_b = memory::pop();
   addr address_a = memory::pop();
   addr destination = memory::pop();
   string text_a = utility::string_pick(address_a);
   string text_b = utility::string_pick(address_b);
   string result = text_a + text_b;
   vector<fpu> packed_pascal = utility::string_to_pascal(result);
   s32 buffer_size = ram_local_fpu[destination - 1];
   u32 limit = min(cast(u32, packed_pascal.size()), cast(u32, buffer_size));
   for (u32 i = 0; i < limit; i++) {ram_local_fpu[destination + i] = packed_pascal[i];}
   opfunc::push(destination);
   OPDONE;
  }

  addr sub(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(4)
   s32 length = memory::pop();
   s32 start = memory::pop();
   addr source = memory::pop();
   addr destination = memory::pop();
   string text = utility::string_pick(source);
   string result = text.substr(start, length);
   vector<fpu> packed_pascal = utility::string_to_pascal(result);
   s32 buffer_size = ram_local_fpu[destination - 1];
   u32 limit = min(cast(u32, packed_pascal.size()), cast(u32, buffer_size));
   for (u32 i = 0; i < limit; i++) {ram_local_fpu[destination + i] = packed_pascal[i];}
   opfunc::push(destination);
   OPDONE;
  }
 }

 MODULE(
  module::add("string", "differ", wrap::differ, 2);
  module::add("string", "order", wrap::order, 2);
  module::add("string", "to_n", wrap::to_n, 1);
  module::add("string", "from_n", wrap::from_n, 2);
  module::add("string", "add", wrap::add, 3);
  module::add("string", "sub", wrap::sub, 4);
 )
}