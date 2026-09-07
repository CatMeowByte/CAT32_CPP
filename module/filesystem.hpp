#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace filesystem {
 const string& get_root();

 // content
 vector<octo> read(const string& path, u32 offset, u32 length);
 u8 overwrite(const string& path, u32 offset, const vector<octo>& data);
 u8 insert(const string& path, u32 offset, const vector<octo>& data);
 u8 delete_byte(const string& path, u32 offset, u32 length);

 // structure
 u8 type(const string& path);
 u32 size(const string& path);
 u8 create(const string& path);
 u8 move(const string& source, const string& destination, bool duplicate);
 u8 remove(const string& path);
 u32 list_count(const string& path);
 string list_index(const string& path, u32 index);
}