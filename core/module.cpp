#include "core/module.hpp"

namespace module {
 vector<Module> table;

 void add(const string& name, fnp function, u8 args_count, vector<fpu> args_default) {
  for (u32 i = 0; i < table.size(); i++) {
   if (table[i].name == name) {
    table[i].function = function;
    table[i].args_count = args_count;
    table[i].args_default = args_default;
    return;
   }
  }
  table.push_back({name, function, args_count, args_default});
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

 const string& get_name(fpu id) {
  return table[id].name;
 }
}