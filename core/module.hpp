#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

// module
// convert registered module opcode in the /module

namespace module {
 struct Module {
  string name;
  fnp function;
 };

 extern vector<Module> table;

 void add(const string& name, fnp function);
 s32 get_index(const string& name);
 bool exist(const string& name);
 const string& get_name(elem id);
};

