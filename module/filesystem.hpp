#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace filesystem {
 vector<octo> read(const string& path, u32 offset, u32 length);
 void write(const string& path, u32 offset, const vector<octo>& data);
 u32 list_count(const string& path);
 string list_index(const string& path, u32 index);
 bool exist(const string& path);
 bool is_dir(const string& path);
 u32 size(const string& path);
 void make_dir(const string& path);
 void remove(const string& path);
 void load(const string& path);
 void run();
}