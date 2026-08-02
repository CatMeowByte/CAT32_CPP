#include "core/constant.hpp"
#include "core/interpreter.hpp"
#include "core/kernel.hpp"
#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"
#include "core/utility.hpp"
#include "module/filesystem.hpp"

namespace kernel {
 void run_event(Event handler) {
  using namespace memory::vm::global::constant;
  active::logic->counter = cast(u8, handler);
  while (active::logic->counter < active::logic->writer) {interpreter::step();}
 }

 void run(const string& path, u8 index) {
  cout << "\n\nLoad: " << path << endl;
  cout << "as process " << cast(u32, index) << endl;
  active::index(index);

  // clear
  {using namespace memory::vm::process; memset(memory::raw + p0_address + (index * p0_size), 0, p0_size);}

  active::logic->writer = cast(u8, kernel::Event::Load) - 1;
  active::logic->slotter = cast(double, memory::vm::process::p0::logic::code_length) / sizeof(fpu); // last of code (in slot)

  memory::unaligned_16_write(active::logic->code_octo + cast(u8, kernel::Event::Init) + 1, FARLAND);
  memory::unaligned_16_write(active::logic->code_octo + cast(u8, kernel::Event::Step) + 1, FARLAND);
  memory::unaligned_16_write(active::logic->code_octo + cast(u8, kernel::Event::Draw) + 1, FARLAND);

  string full_path = filesystem::get_root() + path;
  ifstream file(full_path);
  if (!file) {cerr << "Failed to open file." << endl; return;}

  string line_buffer;

  enum class LineState : u8 {Header, Code, Quote, Data};
  LineState line_state = LineState::Code;

  octo* data_target = nullptr;
  u32 data_index = 0;

  bool is_whitespace_only = false;
  bool is_string_clean = false;
  bool was_newline = false;

  string line;
  while (getline(file, line)) {
   bool is_quote_opened = false;

   is_whitespace_only = true;

   if (line_state == LineState::Data) {
    if (line == "\"") {
     data_target = nullptr;
    }
    else if (data_target) {
     for (u32 i = 0; i < line.size(); i += 2) {
      data_target[data_index++] = ((line[i] & 0xF) + (line[i] >> 6) * 9) << 4 | ((line[i + 1] & 0xF) + (line[i + 1] >> 6) * 9);
     }
    }
    continue;
   }
   else if (line_state == LineState::Code && was_newline) {
    if (line == "str XDB475AEFX = x\"") {
     line_state = LineState::Data;
     data_target = active::local->sprite;
     data_index = 0;
     continue;
    }
    // other brach later
   }

   // per character
   for (u32 pos = 0; pos < line.size();) {

    // substitute UTF-8 to CAT-32 character map
    u8 c = line[pos];
    u8 advance = 1;
    if (c >= 0x80) {
     u32 ordinal = c;
     if ((c & 0xE0) == 0xC0 && pos + 1 < line.size()) {ordinal = ((c & 0x1F) << 6) | (line[pos+1] & 0x3F); advance = 2;}
     else if ((c & 0xF0) == 0xE0 && pos + 2 < line.size()) {ordinal = ((c & 0x0F) << 12) | ((line[pos+1] & 0x3F) << 6) | (line[pos+2] & 0x3F); advance = 3;}
     else if ((c & 0xF8) == 0xF0 && pos + 3 < line.size()) {ordinal = ((c & 0x07) << 18) | ((line[pos+1] & 0x3F) << 12) | ((line[pos+2] & 0x3F) << 6) | (line[pos+3] & 0x3F); advance = 4;}
     c = 31;
     for (u8 i = 0; i < 31; i++) {if (font_special[i] == ordinal) {c = i; break;}}
    }
    pos += advance;

    if (c == '"') {
     u32 backslash_count = 0;
     for (s32 i = line_buffer.size() - 1; i >= 0 && line_buffer[i] == '\\'; i--) {backslash_count++;}
     if (backslash_count % 2 == 0) {
      if (line_state != LineState::Quote) {
       line_state = LineState::Quote;
       is_string_clean = true;
       is_quote_opened = true;
      }
      else {
       while (is_whitespace_only && !is_quote_opened && line_buffer.size() >= 2) {
        if (line_buffer.back() == 'n' && line_buffer[line_buffer.size()-2] == '\\') {
         line_buffer.pop_back();
         line_buffer.pop_back();
         break;
        }
        line_buffer.pop_back();
       }
       line_state = LineState::Code;
      }
     }
    }
    else if (c != ' ' && c != '\t') {
     is_whitespace_only = false;
     is_string_clean = false;
    }
    line_buffer += c;
   }

   // do line buffer
   if (line_state == LineState::Quote) {
    if (is_string_clean && is_quote_opened) {
     while (!line_buffer.empty() && (line_buffer.back() == ' ' || line_buffer.back() == '\t')) {line_buffer.pop_back();}
    }
    else {line_buffer += "\\n";}
   }
   else if (!is_whitespace_only) {
    vector<vector<string>> tokens = interpreter::tokenize(line_buffer);
    interpreter::compile(tokens);
    line_buffer.clear();
   }

   was_newline = is_whitespace_only;
  }

  // dedent hack
  // required to close all scope and unpatched jump
  // which happen in dedent logic
  interpreter::compile(interpreter::tokenize("return"));

  active::logic->stacker = active::logic->slotter; // after slotter filled

  // cleanup
  interpreter::reset();

  // statistic
  u32 code_total = memory::vm::process::p0::logic::code_length;
  u32 slot_total = code_total / sizeof(fpu);
  u32 slotter_index = cast(u32, active::logic->slotter.i());
  u32 bytecode_bytes = cast(u32, active::logic->writer.a());
  u32 allocated_bytes = (slot_total - slotter_index) * sizeof(fpu);
  u32 gap_bytes = slotter_index * sizeof(fpu) - bytecode_bytes;
  cout << endl;
  cout << "Bytecode: " << bytecode_bytes << " bytes (" << (bytecode_bytes * 100 / code_total) << "%)" << endl;
  cout << "Allocate: " << (allocated_bytes / sizeof(fpu)) << " slots (" << (allocated_bytes * 100 / code_total) << "%)" << endl;
  cout << "Available: " << gap_bytes << " bytes / " << (gap_bytes / sizeof(fpu)) << " slots (" << (gap_bytes * 100 / code_total) << "%)" << endl;

  active::logic->code_octo[cast(u8, kernel::Event::Init)] = op::jump;
  active::logic->code_octo[cast(u8, kernel::Event::Step)] = op::jump;
  active::logic->code_octo[cast(u8, kernel::Event::Draw)] = op::jump;

  cout << "\n\nRun:" << endl;
  kernel::run_event(kernel::Event::Load);
  kernel::run_event(kernel::Event::Init);
 }

 namespace wrap {
  OPCODE(run, {
   u8 index = memory::pop().i();
   address_logic address_path = memory::pop().a();
   string string_path = utility::string_pick(address_path);
   kernel::run(string_path, index);
  })
 }

 MODULE(
  module::add("", "run", wrap::run, 2, {0});
 )
}