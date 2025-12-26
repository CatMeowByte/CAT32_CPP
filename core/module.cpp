#include "core/module.hpp"
#include "core/utility.hpp"

namespace module {
 Table table;

 void add(const string& space, const string& name, fnp function, u8 args_count, vector<fpu> args_default) {
  u32 hash_key = ((utility::hash(space.c_str()) & 0xFFFF) << 16) | (utility::hash(name.c_str()) & 0xFFFF);
  Module module;
  module.name = name;
  module.function = function;
  module.args_count = args_count;
  module.args_default = args_default;
  Table::instance()[hash_key] = module;
 }

 bool exist(u32 hash) {
  return Table::instance().count(hash) > 0;
 }

 const string& get_name(u32 hash) {
  return table[hash].name;
 }
}