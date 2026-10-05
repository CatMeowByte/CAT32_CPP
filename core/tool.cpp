#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"
#include "core/tool.hpp"

namespace tool {
 namespace convert {
  double hex_to_number(const string& text) {
   u64 dot = text.find('.');
   string hex = "";
   if (dot == string::npos) {
    hex += string(8 - hex.length(), '0');
    hex = text.substr(2) + hex;
   } else {
    hex += text.substr(dot + 1);
    hex += string(8 - hex.length(), '0');
    hex = text.substr(2, dot - 2) + hex;
   }
   hex = string(16 - hex.length(), '0') + hex;
   double value = stoll(hex, nullptr, 16);
   return value / double(1ULL << 32);
  }

  double bin_to_number(const string& text) {
   u64 dot = text.find('.');
   string bin = "";
   if (dot == string::npos) {
    bin += string(32 - bin.length(), '0'); // fractional pad
    bin = text.substr(2) + bin;
   } else {
    bin += text.substr(dot + 1);
    bin += string(32 - bin.length(), '0');
    bin = text.substr(2, dot - 2) + bin;
   }
   bin = string(64 - bin.length(), '0') + bin;
   u64 value = bin.empty() ? 0ULL : stoull(bin, nullptr, 2);
   return value / double(1ULL << 32);
  }
 }

 namespace text {
  bool is_number(const string& text) {
   if (text.empty()) {return false;}
   u32 i = 0;
   bool has_digit = false;
   bool has_dot = false;

   if (text[0] == '-' || text[0] == '+') i++;

   for (; i < text.length(); i++) {
    if (isdigit(text[i])) {
     has_digit = true;
     continue;
    }
    if (text[i] == '.' && !has_dot) {
     has_dot = true;
     continue;
    }
    return false;
   }
   return has_digit;
  }

  bool is_identifier(const string& text) {
   if (text.empty()) {return false;}
   if (!isalpha(text[0]) && text[0] != '_' && text[0] != '.') {return false;}
   for (char c : text) {if (!isalnum(c) && c != '_' && c != '.') {return false;}}
   return true;
  }

  bool is_hex(const string& text) {
   if (text.size() < 3 || text[0] != '0' || (text[1] != 'x' && text[1] != 'X')) {return false;}
   bool has_dot = false;
   for (size_t i = 2; i < text.size(); i++) {
    char c = text[i];
    if (c == '.') {
     if (has_dot) {return false;}
     has_dot = true;
     continue;
    }
    bool is_digit = (c >= '0' && c <= '9');
    bool is_lower = (c >= 'a' && c <= 'f');
    bool is_upper = (c >= 'A' && c <= 'F');
    if (!(is_digit || is_lower || is_upper)) {return false;}
   }
   return true;
  }

  bool is_bin(const string& text) {
   if (text.size() < 3) {return false;}
   if (!(text[0] == '0' && (text[1] == 'b' || text[1] == 'B'))) {return false;}
   bool has_dot = false, has_digit = false;
   for (u32 i = 2; i < text.size(); i++) {
    char c = text[i];
    if (c == '.') {if (has_dot) {return false;} has_dot = true; continue;}
    if (c == '0' || c == '1') {has_digit = true; continue;}
    return false;
   }
   return has_digit;
  }

  string remove_trailing(double value) {
   string s = to_string(value);
   s.erase(s.find_last_not_of('0') + 1, string::npos); // remove trailing zeros
   if(s.back()=='.') {s.pop_back();} // remove trailing dot
   return s;
  }

  string pick(slot_logic address) {
   string out;
   u16 length = tool::stripe::get_len(address);
   for (u16 i = 0; i < length; i++) {out += cast(char, (active::logic->code_fpu[address + i / 4].r() >> ((i % 4) * 8)) & 0xFF);}
   return out;
  }

  void put(slot_logic address, const string& text) {
   u16 cap = tool::stripe::get_cap(address);
   u16 length = min(text.size(), cap * sizeof(fpu));
   for (u16 i = 0; i < length; i++) {active::logic->code_octo[address * sizeof(fpu) + i] = text[i];}
   tool::stripe::set_len(address, length);
  }
 }

 namespace wrap {
  OPCODE(see, {
   fpu literal_value = memory::pop();

   // format hex
   ostringstream hex_out;
   hex_out.setf(ios::uppercase);
   hex_out << hex;
   hex_out << setw(8);
   hex_out << setfill('0');
   hex_out << literal_value.r();
   string hex_string = hex_out.str();

   u8 dot_position = fpu::WIDTH / 4;
   string fixed_hex = hex_string.substr(0, 8 - dot_position) + "." + hex_string.substr(8 - dot_position);

   // format float
   string decimal_string = tool::text::remove_trailing(literal_value.d());

   cout << "SEE(" << decimal_string << ") = " << fixed_hex << " | " << literal_value.r() << "" << endl;
  })
 }

 MODULE(
  module::add("", "see", wrap::see, 1);
 )
}