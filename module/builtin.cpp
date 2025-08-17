#include "core/memory.hpp"
#include "module/builtin.hpp"

namespace builtin {
 void add(const string& name, fnp function) {
  for (u32 i = 0; i < table.size(); i++) {
   if (table[i].name == name) {table[i].function = function; return;}
  }
  table.push_back({name, function});
 }

 s32 get_index(const string& name) {
  for (u32 i = 0; i < table.size(); i++) {
   if (table[i].name == name) {return i;}
  }
  return -1;
 }

 bool exist(const string& name) {
  return get_index(name) >= 0;
 }

 const string& get_name(elem id) {
  return table[id].name;
 }
}