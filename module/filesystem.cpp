
#include "core/constant.hpp"
#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"
#include "core/utility.hpp"
#include "module/filesystem.hpp"

#include <sys/stat.h>
#if defined(_WIN32)
#include <direct.h>
#include <io.h>
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

 // content

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

 u8 write(const string& path, u32 offset, const vector<octo>& data, bool is_replace) {
  string full_path = get_root() + path;
  FILE* file = fopen(full_path.c_str(), "r+b");
  if (!file) {return 1;}
  fseek(file, 0, SEEK_END);
  u32 file_size = cast(u32, ftell(file));
  if (offset > file_size) {offset = file_size;}
  fseek(file, offset, SEEK_SET);
  if (is_replace) {
   fwrite(data.data(), 1, data.size(), file);
  }
  else {
   constexpr u32 buffer_size = 4 * 1024;
   vector<octo> buffer(buffer_size);
   u32 tail = file_size - offset;
   while (tail) {
    u32 chunk = min(buffer_size, tail);
    u32 read_at = offset + tail - chunk;
    u32 write_at = read_at + cast(u32, data.size());
    fseek(file, read_at, SEEK_SET);
    fread(buffer.data(), 1, chunk, file);
    fseek(file, write_at, SEEK_SET);
    fwrite(buffer.data(), 1, chunk, file);
    tail -= chunk;
   }
   fseek(file, offset, SEEK_SET);
   fwrite(data.data(), 1, data.size(), file);
  }
  fclose(file);
  return 0;
 }

 u8 delete_byte(const string& path, u32 offset, u32 length) {
  string full_path = get_root() + path;
  FILE* file = fopen(full_path.c_str(), "r+b");
  if (!file) {return 1;}
  fseek(file, 0, SEEK_END);
  u32 file_size = cast(u32, ftell(file));
  if (offset > file_size) {offset = file_size;}
  if (length > file_size - offset) {length = file_size - offset;}
  u32 tail_start = offset + length;
  u32 tail_size = file_size - tail_start;
  vector<octo> tail(tail_size);
  if (tail_size) {fseek(file, tail_start, SEEK_SET); fread(tail.data(), 1, tail_size, file);}
  fseek(file, offset, SEEK_SET);
  if (tail_size) {fwrite(tail.data(), 1, tail_size, file);}
  #if defined(_WIN32)
   _chsize(_fileno(file), cast(long, offset + tail_size));
  #elif defined(__linux__)
   ftruncate(fileno(file), cast(off_t, offset + tail_size));
  #else
   #error "Unsupported platform"
  #endif
  fclose(file);
  return 0;
 }

 // structure

 u8 type(const string& path) {
  string full_path = get_root() + path;
  #if defined(_WIN32)
   struct _stat buffer;
   if (_stat(full_path.c_str(), &buffer) != 0) {return 0;}
   if (buffer.st_mode & _S_IFDIR) {return 2;}
   return 1;
  #elif defined(__linux__)
   struct stat buffer;
   if (stat(full_path.c_str(), &buffer) != 0) {return 0;}
   if (S_ISDIR(buffer.st_mode)) {return 2;}
   return 1;
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

 u8 create(const string& path) {
  string current = get_root();
  if (!current.empty() && current[current.size()-1] == '/') {current.pop_back();}
  u32 segment_start = 0;
  for (u32 i = 0; i <= path.size(); i++) {
   if (i == path.size() || path[i] == '/') {
    if (i > segment_start) {
     current += "/" + path.substr(segment_start, i - segment_start);
     if (i == path.size()) {
      #if defined(_WIN32)
       struct _stat buffer;
       if (_stat(current.c_str(), &buffer) == 0) {return 1;}
      #elif defined(__linux__)
       struct stat buffer;
       if (stat(current.c_str(), &buffer) == 0) {return 1;}
      #else
       #error "Unsupported platform"
      #endif
      FILE* file = fopen(current.c_str(), "wb");
      if (file) {fclose(file);}
     }
     else {
      #if defined(_WIN32)
       struct _stat buffer;
       if (_stat(current.c_str(), &buffer) == 0) {
        if (!(buffer.st_mode & _S_IFDIR)) {return 1;}
       }
       else if (_mkdir(current.c_str()) != 0) {return 1;}
      #elif defined(__linux__)
       struct stat buffer;
       if (stat(current.c_str(), &buffer) == 0) {
        if (!S_ISDIR(buffer.st_mode)) {return 1;}
       }
       else if (::mkdir(current.c_str(), 0755) != 0) {return 1;}
      #else
       #error "Unsupported platform"
      #endif
     }
    }
    segment_start = i + 1;
   }
  }
  return 0;
 }

 u8 move(const string& source, const string& destination, bool duplicate) {
  constexpr u32 buffer_size = 4 * 1024;
  string clean_source = source;
  while (clean_source.size() > 1 && clean_source[clean_source.size()-1] == '/') {clean_source.pop_back();}
  string clean_destination = destination;
  while (clean_destination.size() > 1 && clean_destination[clean_destination.size()-1] == '/') {clean_destination.pop_back();}
  if (clean_source == "/") {return 1;}
  if (clean_destination.size() > clean_source.size() && clean_destination.compare(0, clean_source.size(), clean_source) == 0 && clean_destination[clean_source.size()] == '/') {return 1;}
  u8 entry_type = type(clean_source);
  if (entry_type == 0) {return 1;}
  if (type(clean_destination) != 0) {return 1;}
  string full_source = get_root() + clean_source;
  string full_destination = get_root() + clean_destination;
  if (duplicate) {
   if (entry_type == 1) {
    FILE* src = fopen(full_source.c_str(), "rb");
    if (!src) {return 1;}
    FILE* dst = fopen(full_destination.c_str(), "wb");
    if (!dst) {fclose(src); return 1;}
    vector<octo> buffer(buffer_size);
    u32 count;
    while ((count = cast(u32, fread(buffer.data(), 1, buffer_size, src)))) {fwrite(buffer.data(), 1, count, dst);}
    fclose(src);
    fclose(dst);
    return 0;
   }
   if (create(clean_destination + "/")) {return 1;}
   u32 child_count = list_count(clean_source);
   for (u32 i = 0; i < child_count; i++) {
    string name = list_index(clean_source, i);
    if (move(clean_source + "/" + name, clean_destination + "/" + name, true)) {return 1;}
   }
   return 0;
  }
  else {
   if (std::rename(full_source.c_str(), full_destination.c_str()) != 0) {return 1;}
   return 0;
  }
 }

 u8 remove(const string& path) {
  string clean = path;
  while (clean.size() > 1 && clean[clean.size()-1] == '/') {clean.pop_back();}
  u8 entry_type = type(clean);
  if (entry_type == 0) {return 1;}
  string full_path = get_root() + clean;
  if (entry_type == 1) {
   if (std::remove(full_path.c_str()) != 0) {return 1;}
   return 0;
  }
  #if defined(_WIN32)
   WIN32_FIND_DATAA find_data;
   HANDLE handle = FindFirstFileA((full_path + "/*").c_str(), &find_data);
   if (handle == INVALID_HANDLE_VALUE) {return 1;}
   do {
    string name = find_data.cFileName;
    if (name == "." || name == "..") {continue;}
    if (remove(clean + "/" + name)) {FindClose(handle); return 1;}
   } while (FindNextFileA(handle, &find_data));
   FindClose(handle);
   if (_rmdir(full_path.c_str()) != 0) {return 1;}
   return 0;
  #elif defined(__linux__)
   DIR* dir = opendir(full_path.c_str());
   if (!dir) {return 1;}
   struct dirent* entry;
   while ((entry = readdir(dir))) {
    string name = entry->d_name;
    if (name == "." || name == "..") {continue;}
    if (remove(clean + "/" + name)) {closedir(dir); return 1;}
   }
   closedir(dir);
   if (rmdir(full_path.c_str()) != 0) {return 1;}
   return 0;
  #else
   #error "Unsupported platform"
  #endif
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

 namespace wrap {
  // content

  OPCODE(read, {
   u32 length = memory::pop().r();
   u32 offset = memory::pop().r();
   address_logic address_path = memory::pop().a();
   address_logic address_destination = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   vector<octo> data = filesystem::read(string_path, offset, length);
   s32 buffer_size = active::logic->code_fpu[address_destination - 1].i();
   u32 byte_capacity = buffer_size * sizeof(fpu);
   u32 bytes_to_copy = min(cast(u32, data.size()), byte_capacity);
   memcpy(&active::logic->code_fpu[address_destination], data.data(), bytes_to_copy);
  })

  OPCODE(readline, {
   u32 offset = memory::pop().r();
   address_logic address_path = memory::pop().a();
   address_logic address_destination = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   s32 buffer_size = active::logic->code_fpu[address_destination - 1].i();
   u32 byte_capacity = (buffer_size - 1) * sizeof(fpu);
   vector<octo> data = filesystem::read(string_path, offset, byte_capacity);
   u32 data_size = cast(u32, data.size());
   u32 line_length = 0;
   while (line_length < data_size && data[line_length] != '\n') {line_length = line_length + 1;}
   memcpy(&active::logic->code_fpu[address_destination + 1], data.data(), data_size);
   active::logic->code_fpu[address_destination] = fpu(line_length);
   memory::push(fpu::raw(offset + line_length + (line_length < data_size)));
  })

  OPCODE(write, {
   bool is_replace = memory::pop();
   bool is_string = memory::pop();
   u32 length = memory::pop().r();
   u32 offset = memory::pop().r();
   address_logic address_path = memory::pop().a();
   address_logic address_source = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   s32 buffer_size = active::logic->code_fpu[address_source - 1].i();
   u32 byte_capacity = (buffer_size - is_string) * sizeof(fpu);
   u32 byte_count = min(length, byte_capacity);
   if (is_string) {byte_count = min(byte_count, cast(u32, active::logic->code_fpu[address_source].i()));}
   vector<octo> data(byte_count);
   memcpy(data.data(), &active::logic->code_fpu[address_source + is_string], byte_count);
   u8 result = filesystem::write(string_path, offset, data, is_replace);
   memory::push(result);
  })

  OPCODE(delete_byte, {
   u32 length = memory::pop().r();
   u32 offset = memory::pop().r();
   address_logic address_path = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   u8 result = filesystem::delete_byte(string_path, offset, length);
   memory::push(result);
  })

  // structure

  OPCODE(type, {
   address_logic address_path = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   u8 result = filesystem::type(string_path);
   memory::push(result);
  })

  OPCODE(size, {
   address_logic address_path = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   u32 result = filesystem::size(string_path);
   memory::push(fpu::raw(result));
  })

  OPCODE(create, {
   address_logic address_path = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   u8 result = filesystem::create(string_path);
   memory::push(result);
  })

  OPCODE(move, {
   bool duplicate = memory::pop();
   address_logic address_destination = memory::pop().a();
   address_logic address_source = memory::pop().a();
   string string_source = utility::string_pick(address_source);
   string string_destination = utility::string_pick(address_destination);
   u8 result = filesystem::move(string_source, string_destination, duplicate);
   memory::push(result);
  })

  OPCODE(remove, {
   address_logic address_path = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   u8 result = filesystem::remove(string_path);
   memory::push(result);
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
 }

 MODULE(
  module::add("filesystem", "read", wrap::read, 4);
  module::add("filesystem", "readline", wrap::readline, 3);
  module::add("filesystem", "write", wrap::write, 6, {0, 0});
  module::add("filesystem", "delete", wrap::delete_byte, 3);
  module::add("filesystem", "type", wrap::type, 1);
  module::add("filesystem", "size", wrap::size, 1);
  module::add("filesystem", "create", wrap::create, 1);
  module::add("filesystem", "move", wrap::move, 3);
  module::add("filesystem", "remove", wrap::remove, 1);
  module::add("filesystem", "list_count", wrap::list_count, 1);
  module::add("filesystem", "list_index", wrap::list_index, 3);
 )
}