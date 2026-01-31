#include "core/memory.hpp"
#include "core/module.hpp"
#include "core/opcode.hpp"
#include "core/utility.hpp"
#include "module/video.hpp"

namespace FONT {
 constexpr u8 WIDTH = 4;
 constexpr u8 HEIGHT = 8;
}

namespace video {
 void clear(u8 color) {
  using namespace memory::vm::ram_global;
  memset(framebuffer, (color & 0xF) << 4 | (color & 0xF), framebuffer_size);
 }

 u8 pixel(s32 x, s32 y, s8 color = -1) {
  if (x < 0 || y < 0 || x >= VIDEO::WIDTH || y >= VIDEO::HEIGHT) {return 0;}
  u16 index = y * VIDEO::WIDTH + x;
  u16 byte_pos = index / 2;
  u8 shift = (index % 2 == 0) ? 4 : 0;

  using namespace memory::vm::ram_global;
  u8 byte_old = framebuffer[byte_pos];
  u8 color_old = (byte_old >> shift) & 0xF;
  if (~color && palette[color & 0xF] & 0x80) {framebuffer[byte_pos] = (byte_old & ~(0xF << shift)) | ((color & 0xF) << shift);}
  return color_old;
 }

 void lineh(s32 x, s32 y, s32 length, u8 color) {
  if (y < 0 || y >= VIDEO::HEIGHT || length <= 0) {return;}
  if (x < 0) {length += x; x = 0;}
  if (x + length > VIDEO::WIDTH) {length = VIDEO::WIDTH - x;}
  if (length <= 0) {return;}
  using namespace memory::vm::ram_global;

  if (!(palette[color & 0xF] & 0x80)) {return;}

  u16 index_start = y * VIDEO::WIDTH + x;
  u16 index_end = index_start + length;

  u8 byte_color = (color & 0xF) << 4 | (color & 0xF);

  if (index_start % 2 != 0) {
   u16 pos = index_start / 2;
   framebuffer[pos] = (framebuffer[pos] & 0xF0) | (color & 0xF);
   index_start++;
  }

  if (index_end % 2 != 0) {
   index_end--;
   u16 pos = index_end / 2;
   framebuffer[pos] = (framebuffer[pos] & 0x0F) | ((color & 0xF) << 4);
  }

  if (index_start < index_end) {
   u16 byte_start = index_start / 2;
   u16 byte_len = (index_end - index_start) / 2;
   memset(framebuffer + byte_start, byte_color, byte_len);
  }
 }

 void line(s32 ax, s32 ay, s32 bx, s32 by, u8 color) {
  if (ay == by) {
   lineh(ax, ay, bx - ax + (bx >= ax ? 1 : -1), color);
   return;
  }

  s32 dx = abs(bx - ax);
  s32 dy = abs(by - ay);
  s32 sx = (ax < bx) ? 1 : -1;
  s32 sy = (ay < by) ? 1 : -1;
  s32 err = (dx > dy ? dx : -dy) / 2;
  s32 e2;

  while (true) {
   pixel(ax, ay, color);
   if (ax == bx && ay == by) {break;}
   e2 = err;
   if (e2 > -dx) {err -= dy; ax += sx;}
   if (e2 < dy) {err += dx; ay += sy;}
  }
 }

 void rect(s32 x, s32 y, s32 width, s32 height, u8 color, bool fill) {
  if (width <= 0 || height <= 0) {return;}
  if (x + width <= 0 || y + height <= 0 || x >= VIDEO::WIDTH || y >= VIDEO::HEIGHT) {return;}

  if (x < 0) {width += x; x = 0;}
  if (y < 0) {height += y; y = 0;}
  if (x + width > VIDEO::WIDTH) {width = VIDEO::WIDTH - x;}
  if (y + height > VIDEO::HEIGHT) {height = VIDEO::HEIGHT - y;}
  if (width <= 0 || height <= 0) {return;}

  if (fill) {
   for (s32 i = 0; i < height; i++) {
    lineh(x, y + i, width, color);
   }
  } else {
   lineh(x, y, width, color);
   if (height > 1) {
    lineh(x, y + height - 1, width, color);
    for (s32 i = 1; i < height - 1; i++) {
     pixel(x, y + i, color);
     if (width > 1) {
      pixel(x + width - 1, y + i, color);
     }
    }
   }
  }
 }

 void circle(s32 x, s32 y, s32 radius, u8 color, bool fill) {
  s32 cx = radius;
  s32 cy = 0;
  s32 error = 1 - radius;

  while (cx >= cy) {
   if (fill) {
    lineh(x - cx, y + cy, 2 * cx + 1, color);
    lineh(x - cx, y - cy, 2 * cx + 1, color);
    lineh(x - cy, y + cx, 2 * cy + 1, color);
    lineh(x - cy, y - cx, 2 * cy + 1, color);
   }
   else {
    pixel(x + cx, y + cy, color);
    pixel(x - cx, y + cy, color);
    pixel(x + cx, y - cy, color);
    pixel(x - cx, y - cy, color);
    pixel(x + cy, y + cx, color);
    pixel(x - cy, y + cx, color);
    pixel(x + cy, y - cx, color);
    pixel(x - cy, y - cx, color);
   }

   cy++;
   if (error < 0) {error += 2 * cy + 1;}
   else {cx--; error += 2 * (cy - cx) + 1;}
  }
 }

 void text(s32 x, s32 y, string text, u8 color, u8 background) {
  s32 cursor_x = x;
  s32 cursor_y = y;

  for (u32 string_index = 0; string_index < text.length();) {
   u8* bytes = reinterpret(u8*, &text[string_index]);
   u32 ordinal = bytes[0];
   s32 byte_length = 1;

   if ((bytes[0] & 0xE0) == 0xC0 && (string_index + 1) < text.length()) {
    ordinal = ((bytes[0] & 0x1F) << 6) | (bytes[1] & 0x3F);
    byte_length = 2;
   }
   else if ((bytes[0] & 0xF0) == 0xE0 && (string_index + 2) < text.length()) {
    ordinal = ((bytes[0] & 0x0F) << 12) | ((bytes[1] & 0x3F) << 6) | (bytes[2] & 0x3F);
    byte_length = 3;
   }
   else if ((bytes[0] & 0xF8) == 0xF0 && (string_index + 3) < text.length()) {
    ordinal = ((bytes[0] & 0x07) << 18) | ((bytes[1] & 0x3F) << 12) | ((bytes[2] & 0x3F) << 6) | (bytes[3] & 0x3F);
    byte_length = 4;
   }

   string_index += byte_length;

   if (ordinal == '\n') {
    cursor_x = x;
    cursor_y += FONT::HEIGHT;
    continue;
   }

   if (cursor_x < -FONT::WIDTH || cursor_x >= VIDEO::WIDTH || cursor_y < -FONT::HEIGHT || cursor_y >= VIDEO::HEIGHT) {
    cursor_x += FONT::WIDTH;
    continue;
   }

   s32 character_index = 0;
   if (ordinal >= 0x20 && ordinal <= 0x7E) {character_index = ordinal - 0x20 + 32;}
   else {
    for (s32 special_index = 0; special_index < 32; special_index++) {
     if (font_special[special_index] == ordinal) {character_index = special_index; break;}
    }
   }

   if (!character_index) {cursor_x += FONT::WIDTH; continue;}

   s32 grid_x = (character_index % 16) * FONT::WIDTH;
   s32 grid_y = (character_index / 16) * FONT::HEIGHT;

   for (s32 pixel_y = 0; pixel_y < FONT::HEIGHT; pixel_y++) {
    for (s32 pixel_x = 0; pixel_x < FONT::WIDTH; pixel_x++) {
     s32 bit_position = ((grid_y + pixel_y) * 64 + (grid_x + pixel_x));
     s32 screen_x = cursor_x + pixel_x;
     s32 screen_y = cursor_y + pixel_y;
     if (screen_x < 0 || screen_x >= VIDEO::WIDTH || screen_y < 0 || screen_y >= VIDEO::HEIGHT) {continue;}

     using namespace memory::vm::ram_global;
     u8 pixel_color = font[bit_position / 8] >> (7 - bit_position % 8) & 1 ? color : background;
     pixel(screen_x, screen_y, pixel_color);
    }
   }

   cursor_x += FONT::WIDTH;
  }
 }

 namespace wrap {
  addr clear(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(1)
   u8 color = memory::pop();
   video::clear(color);
   OPDONE;
  }

  addr pixel(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(3)
   u8 color = memory::pop();
   s32 y = memory::pop();
   s32 x = memory::pop();
   u8 color_old = video::pixel(x, y, color);
   opfunc::push(color_old);
   OPDONE;
  }

  addr line(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(5)
   u8 color = memory::pop();
   s32 by = memory::pop();
   s32 bx = memory::pop();
   s32 ay = memory::pop();
   s32 ax = memory::pop();
   video::line(ax, ay, bx, by, color);
   OPDONE;
  }

  addr rect(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(6)
   bool fill = memory::pop();
   u8 color = memory::pop();
   s32 height = memory::pop();
   s32 width = memory::pop();
   s32 y = memory::pop();
   s32 x = memory::pop();
   video::rect(x, y, width, height, color, fill);
   OPDONE;
  }

  addr circle(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(5)
   bool fill = memory::pop();
   u8 color = memory::pop();
   s32 radius = memory::pop();
   s32 y = memory::pop();
   s32 x = memory::pop();
   video::circle(x, y, radius, color, fill);
   OPDONE;
  }

  addr text(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(5)
   u8 background = memory::pop();
   u8 color = memory::pop();
   addr address = memory::pop();
   s32 y = memory::pop();
   s32 x = memory::pop();
   video::text(x, y, utility::string_pick(address), color, background);
   OPDONE;
  }

  addr color(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(2)
   u8 target = memory::pop();
   u8 index = memory::pop();
   using namespace memory::vm::ram_global;
   palette[index & 0xF] = (palette[index & 0xF] & 0x80) | (cast(u8, target) & 0xF);
   OPDONE;
  }

  addr alpha(fpu value) {
   BAIL_UNLESS_STACK_ATLEAST(2)
   bool toggle = memory::pop();
   u8 index = memory::pop();
   using namespace memory::vm::ram_global;
   palette[index & 0xF] = (palette[index & 0xF] & 0x7F) | (toggle ? 0x80 : 0);
   OPDONE;
  }
 }

 MODULE(
  module::add("video", "clear", wrap::clear, 1, {0});
  module::add("video", "pixel", wrap::pixel, 3, {-1});
  module::add("video", "line", wrap::line, 5);
  module::add("video", "rect", wrap::rect, 6, {1});
  module::add("video", "circle", wrap::circle, 5, {1});
  module::add("video", "text", wrap::text, 5, {0});
  module::add("video", "color", wrap::color, 2);
  module::add("video", "alpha", wrap::alpha, 2);
 )
}