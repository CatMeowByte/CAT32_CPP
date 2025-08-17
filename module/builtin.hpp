#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace builtin {
 void add(const string& name, fnp function);
 s32 get_index(const string& name);
 bool exist(const string& name);
 const string& get_name(elem id);
};

