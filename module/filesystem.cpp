
#include "core/interpreter.hpp"
#include <sys/stat.h>
#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#elif defined(__linux__)
#include <dirent.h>
#include <unistd.h>
#endif

#include "core/kernel.hpp"
#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"
#include "core/utility.hpp"
#include "module/filesystem.hpp"

namespace filesystem {
 static struct {
  string path;
  vector<string> items;
 } list_cache;

 static const string& get_root() {
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

 void load(const string& path) {
  using namespace memory::vm::process::app;

  memset(bytecode, 0, bytecode_size);
  memset(ram_local_octo, 0, ram_local_size);

  using namespace ram_local;
  stacker = field_length;
  slotter = cast(s32, (field_address - ram_local_address) / sizeof(fpu));
  writer = 15;

  memory::unaligned_32_write(bytecode + 1, memory::vm::ram_global::constant::sentinel.value);
  memory::unaligned_32_write(bytecode + 6, memory::vm::ram_global::constant::sentinel.value);
  memory::unaligned_32_write(bytecode + 11, memory::vm::ram_global::constant::sentinel.value);


  interpreter::reset();

  string full_path = get_root() + path;
  ifstream file(full_path);
  if (!file) {cerr << "Failed to open file." << endl; return;}

  string line;
  while (getline(file, line)) {
   vector<vector<string>> tokens = interpreter::tokenize(line);
   interpreter::compile(tokens);
  }

  // dedent hack
  interpreter::compile(interpreter::tokenize("return"));

  // block event
  bytecode[cast(u8, kernel::Event::Init)] = op::subret;
  bytecode[cast(u8, kernel::Event::Step)] = op::subret;
  bytecode[cast(u8, kernel::Event::Draw)] = op::subret;
 }

 void run() {

  using namespace memory::vm::process::app;
  bytecode[cast(u8, kernel::Event::Init)] = op::jump;
  bytecode[cast(u8, kernel::Event::Step)] = op::jump;
  bytecode[cast(u8, kernel::Event::Draw)] = op::jump;
  kernel::run_event(kernel::Event::Load);
  kernel::run_event(kernel::Event::Init);
 }

 namespace wrap {
  using namespace memory::vm::process::app;

  addr read(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(4)
   u32 length = memory::pop();
   u32 offset = memory::pop();
   addr address_path = memory::pop();
   addr address_destination = memory::pop();
   string string_path = utility::string_pick(address_path);
   vector<octo> data = filesystem::read(string_path, offset, length);
   s32 buffer_size = ram_local_fpu[address_destination - 1];
   u32 byte_capacity = buffer_size * sizeof(fpu);
   u32 bytes_to_copy = min(cast(u32, data.size()), byte_capacity);
   memcpy(&ram_local_fpu[address_destination], data.data(), bytes_to_copy);
   OPDONE;
  }

  addr write(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(3)
   addr address_source = memory::pop();
   u32 offset = memory::pop();
   addr address_path = memory::pop();
   string string_path = utility::string_pick(address_path);
   s32 source_size = ram_local_fpu[address_source - 1];
   u32 byte_count = source_size * sizeof(fpu);
   vector<octo> data(byte_count);
   memcpy(data.data(), &ram_local_fpu[address_source], byte_count);
   filesystem::write(string_path, offset, data);
   OPDONE;
  }

  addr list_count(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(1)
   addr address_path = memory::pop();
   string string_path = utility::string_pick(address_path);
   u32 count = filesystem::list_count(string_path);
   opfunc::push(count);
   OPDONE;
  }

  addr list_index(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(3)
   u32 index = memory::pop();
   addr address_path = memory::pop();
   addr address_destination = memory::pop();
   string string_path = utility::string_pick(address_path);
   string result = filesystem::list_index(string_path, index);
   vector<fpu> packed_pascal = utility::string_to_pascal(result);
   s32 buffer_size = ram_local_fpu[address_destination - 1];
   u32 limit = min(cast(u32, packed_pascal.size()), cast(u32, buffer_size));
   for (u32 i = 0; i < limit; i++) {ram_local_fpu[address_destination + i] = packed_pascal[i];}
   OPDONE;
  }

  addr exist(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(1)
   addr address_path = memory::pop();
   string string_path = utility::string_pick(address_path);
   bool result = filesystem::exist(string_path);
   opfunc::push(result);
   OPDONE;
  }

  addr is_dir(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(1)
   addr address_path = memory::pop();
   string string_path = utility::string_pick(address_path);
   bool result = filesystem::is_dir(string_path);
   opfunc::push(result);
   OPDONE;
  }

  addr size(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(1)
   addr address_path = memory::pop();
   string string_path = utility::string_pick(address_path);
   u32 result = filesystem::size(string_path);
   opfunc::push(result);
   OPDONE;
  }

  addr make_dir(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(1)
   addr address_path = memory::pop();
   string string_path = utility::string_pick(address_path);
   filesystem::make_dir(string_path);
   OPDONE;
  }

  addr remove(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(1)
   addr address_path = memory::pop();
   string string_path = utility::string_pick(address_path);
   filesystem::remove(string_path);
   OPDONE;
  }

  addr load(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(1)
   addr address_path = memory::pop();
   string string_path = utility::string_pick(address_path);
   filesystem::load(string_path);
   OPDONE;
  }

  addr run(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(1)
   addr address_path = memory::pop();
   // FIXME:
   // first time encountering "reference" default value
   // if value gives the value directly, reference means giving the memory address that point to the value
   // any number SHOULD be valid by the peekpoke rule
   // though it seems like the invalid/nothing is undecided, so its currently take sentinel (-32768 points at the first bytecode)
   // and the current implementation still hasnt considered negative value yet, so currently can not put thing in global ram
   using namespace memory::vm::ram_global::constant;
   if (address_path != cast(addr, sentinel) && ram_local_fpu[address_path] != zero) {
    string string_path = utility::string_pick(address_path);
    filesystem::load(string_path);
   }
   filesystem::run();
   OPDONE;
  }
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
  module::add("", "load", wrap::load, 1);
  module::add("", "run", wrap::run, 1, {memory::vm::ram_global::constant::sentinel});
 )
}