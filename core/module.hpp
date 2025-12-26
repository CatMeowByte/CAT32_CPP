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

 // meyer initialization
 // cpp execute the macro constructor in unpredictable order, maybe even before the table is initialized
 // this ensure the table exist before the constructor use it
 struct Table {
  static hash_map<u32, Module>& instance() {static hash_map<u32, Module> map; return map;}
  Module& operator[](u32 hash) {return instance()[hash];}
 };

 extern Table table;

 void add(const string& space, const string& name, fnp function, u8 args_count, vector<fpu> args_default = {});
 bool exist(u32 hash);
 const string& get_name(u32 hash);
};

// register
#define MODULE(...) namespace {using module::add; struct Register {Register() {__VA_ARGS__}} module_register;}