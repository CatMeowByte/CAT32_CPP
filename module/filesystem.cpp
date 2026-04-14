
#include "core/constant.hpp"
#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"
#include "core/utility.hpp"
#include "module/filesystem.hpp"

#include <sys/stat.h>
#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#elif defined(__linux__)
#include <dirent.h>
#include <unistd.h>
#endif

namespace filesystem {
 static struct {
  string path;
  vector<string> items;
 } list_cache;

 const string& get_root() {
  #if defined(_WIN32)
   static const string root = string(getenv("USERPROFILE")) + "/Documents/CAT32/";
  #elif defined(__linux__)
   static const string root = string(getenv("HOME")) + "/Documents/CAT32/";
  #else
   #error "Unsupported platform"
  #endif
  return root;
 }

 vector<octo> read(const string& path, u32 offset, u32 length) {
  string full_path = get_root() + path;
  vector<octo> data;
  FILE* file = fopen(full_path.c_str(), "rb");
  if (!file) {return data;}
  fseek(file, offset, SEEK_SET);
  data.resize(length);
  u32 bytes_read = fread(data.data(), 1, length, file);
  data.resize(bytes_read);
  fclose(file);
  return data;
 }

 void write(const string& path, u32 offset, const vector<octo>& data) {
  string full_path = get_root() + path;
  FILE* file = fopen(full_path.c_str(), "r+b");
  if (!file) {
   file = fopen(full_path.c_str(), "wb");
   if (!file) {return;}
  }
  fseek(file, offset, SEEK_SET);
  fwrite(data.data(), 1, data.size(), file);
  fclose(file);
 }

 u32 list_count(const string& path) {
  if (path != list_cache.path) {
   list_cache.path = path;
   list_cache.items.clear();
   string full_path = get_root() + path;
   #if defined(_WIN32)
    WIN32_FIND_DATAA find_data;
    HANDLE handle = FindFirstFileA((full_path + "/*").c_str(), &find_data);
    if (handle == INVALID_HANDLE_VALUE) {return 0;}
    do {
     string name = find_data.cFileName;
     if (name != "." && name != "..") {list_cache.items.push_back(name);}
    } while (FindNextFileA(handle, &find_data));
    FindClose(handle);
   #elif defined(__linux__)
    DIR* dir = opendir(full_path.c_str());
    if (!dir) {return 0;}
    struct dirent* entry;
    while ((entry = readdir(dir))) {
     string name = entry->d_name;
     if (name != "." && name != "..") {list_cache.items.push_back(name);}
    }
    closedir(dir);
   #else
    #error "Unsupported platform"
   #endif
  }
  return list_cache.items.size();
 }

 string list_index(const string& path, u32 index) {
  if (path != list_cache.path) {
   list_cache.path = path;
   list_cache.items.clear();
   string full_path = get_root() + path;
   #if defined(_WIN32)
    WIN32_FIND_DATAA find_data;
    HANDLE handle = FindFirstFileA((full_path + "/*").c_str(), &find_data);
    if (handle == INVALID_HANDLE_VALUE) {return "";}
    do {
     string name = find_data.cFileName;
     if (name != "." && name != "..") {list_cache.items.push_back(name);}
    } while (FindNextFileA(handle, &find_data));
    FindClose(handle);
   #elif defined(__linux__)
    DIR* dir = opendir(full_path.c_str());
    if (!dir) {return "";}
    struct dirent* entry;
    while ((entry = readdir(dir))) {
     string name = entry->d_name;
     if (name != "." && name != "..") {list_cache.items.push_back(name);}
    }
    closedir(dir);
   #else
    #error "Unsupported platform"
   #endif
  }
  if (index < list_cache.items.size()) {
   return list_cache.items[index];
  }
  return "";
 }

 bool exist(const string& path) {
  string full_path = get_root() + path;
  #if defined(_WIN32)
   struct _stat buffer;
   return _stat(full_path.c_str(), &buffer) == 0;
  #elif defined(__linux__)
   struct stat buffer;
   return stat(full_path.c_str(), &buffer) == 0;
  #else
   #error "Unsupported platform"
  #endif
 }

 bool is_dir(const string& path) {
  string full_path = get_root() + path;
  #if defined(_WIN32)
   struct _stat buffer;
   if (_stat(full_path.c_str(), &buffer) != 0) {return false;}
   return (buffer.st_mode & _S_IFDIR) != 0;
  #elif defined(__linux__)
   struct stat buffer;
   if (stat(full_path.c_str(), &buffer) != 0) {return false;}
   return S_ISDIR(buffer.st_mode);
  #else
   #error "Unsupported platform"
  #endif
 }

 u32 size(const string& path) {
  string full_path = get_root() + path;
  #if defined(_WIN32)
   struct _stat buffer;
   if (_stat(full_path.c_str(), &buffer) != 0) {return 0;}
   return cast(u32, buffer.st_size);
  #elif defined(__linux__)
   struct stat buffer;
   if (stat(full_path.c_str(), &buffer) != 0) {return 0;}
   return cast(u32, buffer.st_size);
  #else
   #error "Unsupported platform"
  #endif
 }

 void make_dir(const string& path) {
  string full_path = get_root() + path;
  #if defined(_WIN32)
   _mkdir(full_path.c_str());
  #elif defined(__linux__)
   ::mkdir(full_path.c_str(), 0755);
  #else
   #error "Unsupported platform"
  #endif
 }

 void remove(const string& path) {
  string full_path = get_root() + path;
  if (is_dir(path)) {
   #if defined(_WIN32)
    _rmdir(full_path.c_str());
   #elif defined(__linux__)
    rmdir(full_path.c_str());
   #else
    #error "Unsupported platform"
   #endif
  }
  else {
   std::remove(full_path.c_str());
  }
 }

 namespace wrap {
  OPCODE(read, {
   u32 length = memory::pop();
   u32 offset = memory::pop();
   address_logic address_path = memory::pop().a();
   address_logic address_destination = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   vector<octo> data = filesystem::read(string_path, offset, length);
   s32 buffer_size = active::logic->code_fpu[address_destination - 1].i();
   u32 byte_capacity = buffer_size * sizeof(fpu);
   u32 bytes_to_copy = min(cast(u32, data.size()), byte_capacity);
   memcpy(&active::logic->code_fpu[address_destination], data.data(), bytes_to_copy);
  })

  OPCODE(write, {
   address_logic address_source = memory::pop().a();
   u32 offset = memory::pop();
   address_logic address_path = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   s32 source_size = active::logic->code_fpu[address_source - 1].i();
   u32 byte_count = source_size * sizeof(fpu);
   vector<octo> data(byte_count);
   memcpy(data.data(), &active::logic->code_fpu[address_source], byte_count);
   filesystem::write(string_path, offset, data);
  })

  OPCODE(list_count, {
   address_logic address_path = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   u32 count = filesystem::list_count(string_path);
   memory::push(count);
  })

  OPCODE(list_index, {
   u32 index = memory::pop();
   address_logic address_path = memory::pop().a();
   address_logic address_destination = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   string result = filesystem::list_index(string_path, index);
   vector<fpu> packed_pascal = utility::string_to_pascal(result);
   s32 buffer_size = active::logic->code_fpu[address_destination - 1].i();
   u32 limit = min(cast(u32, packed_pascal.size()), cast(u32, buffer_size));
   for (u32 i = 0; i < limit; i++) {active::logic->code_fpu[address_destination + i] = packed_pascal[i];}
  })

  OPCODE(exist, {
   address_logic address_path = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   bool result = filesystem::exist(string_path);
   memory::push(result);
  })

  OPCODE(is_dir, {
   address_logic address_path = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   bool result = filesystem::is_dir(string_path);
   memory::push(result);
  })

  OPCODE(size, {
   address_logic address_path = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   u32 result = filesystem::size(string_path);
   memory::push(result);
  })

  OPCODE(make_dir, {
   address_logic address_path = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   filesystem::make_dir(string_path);
  })

  OPCODE(remove, {
   address_logic address_path = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   filesystem::remove(string_path);
  })
 }

 MODULE(
  module::add("filesystem", "read", wrap::read, 4);
  module::add("filesystem", "write", wrap::write, 3);
  module::add("filesystem", "list_count", wrap::list_count, 1);
  module::add("filesystem", "list_index", wrap::list_index, 3);
  module::add("filesystem", "exist", wrap::exist, 1);
  module::add("filesystem", "is_dir", wrap::is_dir, 1);
  module::add("filesystem", "size", wrap::size, 1);
  module::add("filesystem", "make_dir", wrap::make_dir, 1);
  module::add("filesystem", "remove", wrap::remove, 1);
 )
}