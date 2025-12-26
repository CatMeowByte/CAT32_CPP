#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"
#include "core/utility.hpp"
#include "module/string.hpp"

namespace string_ops {
 s32 differ(const string& str_a, const string& str_b) {
  u32 min_len = (str_a.length() < str_b.length()) ? str_a.length() : str_b.length();

  for (u32 i = 0; i < min_len; i++) {
   u8 char_a = str_a[i];
   u8 char_b = str_b[i];
   if (char_a < char_b) {return -1;}
   if (char_a > char_b) {return 1;}
  }

  if (str_a.length() < str_b.length()) {return -1;}
  if (str_a.length() > str_b.length()) {return 1;}
  return 0;
 }

 s32 order(const string& str_a, const string& str_b) {
  u32 pos_a = 0;
  u32 pos_b = 0;

  while (pos_a < str_a.length() && pos_b < str_b.length()) {
   u8 char_a = str_a[pos_a];
   u8 char_b = str_b[pos_b];

   bool digit_a = (char_a >= '0' && char_a <= '9');
   bool digit_b = (char_b >= '0' && char_b <= '9');

   if (digit_a && digit_b) {
    s32 num_a = 0;
    s32 num_b = 0;

    while (pos_a < str_a.length()) {
     u8 c = str_a[pos_a];
     if (c < '0' || c > '9') {break;}
     num_a = num_a * 10 + (c - '0');
     pos_a++;
    }

    while (pos_b < str_b.length()) {
     u8 c = str_b[pos_b];
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

  if (pos_a < str_a.length()) {return 1;}
  if (pos_b < str_b.length()) {return -1;}
  return 0;
 }

 namespace wrap {
  addr differ(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(2)
   fpu addr_b = memory::pop();
   fpu addr_a = memory::pop();
   string str_a = utility::string_pick(addr_a);
   string str_b = utility::string_pick(addr_b);
   opfunc::push(string_ops::differ(str_a, str_b));
   OPDONE;
  }

  addr order(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(2)
   fpu addr_b = memory::pop();
   fpu addr_a = memory::pop();
   string str_a = utility::string_pick(addr_a);
   string str_b = utility::string_pick(addr_b);
   opfunc::push(string_ops::order(str_a, str_b));
   OPDONE;
  }
 }

 MODULE(
  module::add("string", "differ", wrap::differ, 2);
  module::add("string", "order", wrap::order, 2);
 )
}