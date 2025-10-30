#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

// module
// convert registered module opcode in the /module

namespace module {
 struct Module {
  string name;
  fnp function;
  u8 args_count;
  vector<fpu> args_default;
 };

 extern vector<Module> table;

 void add(const string& name, fnp function, u8 args_count, vector<fpu> args_default = {});
 s32 get_index(const string& name);
 bool exist(const string& name);
 const string& get_name(fpu id);
};

