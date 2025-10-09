#include "core/memory.hpp"

// TODO:
// possibly move the variable definition to the appropriate module/file that control it
// or not, why dont keep things organized by just clump it in one predictable searchable place
// just define it here

namespace symbol {
 vector<Data> table;

 s32 get_index_reverse(const string& name) {
  for (u32 i = table.size(); i-- > 0;) {
   if (symbol::table[i].name == name) {return i;}
  }
  return -1;
 }

 bool exist(const string& name) {
  return get_index_reverse(name) >= 0;
 }

 Data& get(const string& name) {
  return table[get_index_reverse(name)];
 }
}

namespace scope {
 vector<Frame> stack;
 u8 previous = 0;

 addr last_jump_operand = SENTINEL;
 addr last_line_start = SENTINEL;
 Type last_line_scope_set = Type::Generic;
}