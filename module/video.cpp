#include "core/memory.hpp"
#include "core/opcode.hpp"
#include "core/utility.hpp"
#include "module/builtin.hpp"
#include "module/font.hpp"
#include "module/video.hpp"
#include "library/sdl.hpp"

static u8 buffer[VIDEO::WIDTH * VIDEO::HEIGHT >> 1];

static s32 cam_x = 0;
static s32 cam_y = 0;
static bool text_wrap = false;

namespace video {
 u8 pixel(s32 x, s32 y) {
  if (x < 0 || y < 0 || x >= VIDEO::WIDTH || y >= VIDEO::HEIGHT) {return 0;}
  u16 index = y * VIDEO::WIDTH + x;
  u16 byte_pos = index / 2;
  u8 shift = (index % 2 == 0) ? 4 : 0;
  return (buffer[byte_pos] >> shift) & 0xF;
 }

 void pixel(s32 x, s32 y, u8 color) {
  if (x < 0 || y < 0 || x >= VIDEO::WIDTH || y >= VIDEO::HEIGHT) {return;}
  u16 index = y * VIDEO::WIDTH + x;
  u16 byte_pos = index / 2;
  u8 shift = (index % 2 == 0) ? 4 : 0;
  buffer[byte_pos] &= ~(0xF << shift);
  buffer[byte_pos] |= (color & 0xF) << shift;
 }

 void lineh(s32 x, s32 y, s32 length, u8 color) {
  if (y < 0 || y >= VIDEO::HEIGHT || length <= 0) {return;}
  if (x < 0) {length += x; x = 0;}
  if (x + length > VIDEO::WIDTH) {length = VIDEO::WIDTH - x;}
  if (length <= 0) {return;}

  u16 start_index = y * VIDEO::WIDTH + x;
  u16 end_index = start_index + length;

  u8 cbyte = (color & 0xF) << 4 | (color & 0xF);

  if (start_index % 2 != 0) {
   buffer[start_index / 2] &= 0xF0;
   buffer[start_index / 2] |= color & 0xF;
   start_index++;
  }

  if (end_index % 2 != 0) {
   end_index--;
   buffer[end_index / 2] &= 0x0F;
   buffer[end_index / 2] |= (color & 0xF) << 4;
  }

  if (start_index < end_index) {
   u16 byte_start = start_index / 2;
   u16 byte_len = (end_index - start_index) / 2;
   memset(buffer + byte_start, cbyte, byte_len);
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

 void text(s32 x, s32 y, string text, u8 color, u8 background) {
  x -= cam_x;
  y -= cam_y;

  s32 current_x = x;
  s32 current_y = y;

  for (u32 i = 0; i < text.length();) {
   u8* ch = reinterpret(u8*, &text[i]);
   u32 ordinal = ch[0];
   s32 chlen = 1;

   if ((ch[0] & 0xE0) == 0xC0 && (i + 1) < text.length()) {
     ordinal = ((ch[0] & 0x1F) << 6) | (ch[1] & 0x3F);
     chlen = 2;
   } else if ((ch[0] & 0xF0) == 0xE0 && (i + 2) < text.length()) {
     ordinal = ((ch[0] & 0x0F) << 12) | ((ch[1] & 0x3F) << 6) | (ch[2] & 0x3F);
     chlen = 3;
   } else if ((ch[0] & 0xF8) == 0xF0 && (i + 3) < text.length()) {
     ordinal = ((ch[0] & 0x07) << 18) | ((ch[1] & 0x3F) << 12) | ((ch[2] & 0x3F) << 6) | (ch[3] & 0x3F);
     chlen = 4;
   }

   i += chlen;

   if (ordinal == '\n') {
    current_x = x;
    current_y += FONT::HEIGHT;
    continue;
   }

   if (text_wrap && current_x >= VIDEO::WIDTH) {
    if (ordinal == ' ') {continue;}
    current_x = x;
    current_y += FONT::HEIGHT;
   }

   if (current_x < -FONT::WIDTH || current_x >= VIDEO::WIDTH || current_y < -FONT::HEIGHT || current_y >= VIDEO::HEIGHT) {
    current_x += FONT::WIDTH;
    continue;
   }

   u32 bits = (ordinal == ' ') ? 0 : CHARACTER(ordinal);

   for (s32 py = 0; py < FONT::HEIGHT; py++) {
    for (s32 px = 0; px < FONT::WIDTH; px++) {
     bool on = (bits >> (py * FONT::WIDTH + px)) & 1;
     s32 tx = current_x + px;
     s32 ty = current_y + py;
     if (tx < 0 || tx >= VIDEO::WIDTH || ty < 0 || ty >= VIDEO::HEIGHT) {continue;}
     pixel(tx, ty, on ? color : background);
    }
   }

   current_x += FONT::WIDTH;
  }
 }

 void clear(u8 color) {
  memset(buffer, (color & 0xF) << 4 | (color & 0xF), sizeof(buffer));
 }

 void flip() {
  sdl::flip(buffer);
 }

 namespace wrap {
  addr clear(fpu) {
   BAIL_UNLESS_STACK_ATLEAST(1)
   s32 color = cast(s32, memory[stacker++]);
   video::clear(color);
   return SENTINEL;
  }

  addr pixel(fpu) {
   BAIL_UNLESS_STACK_ATLEAST(3)
   s32 color = cast(s32, memory[stacker++]);
   s32 y = cast(s32, memory[stacker++]);
   s32 x = cast(s32, memory[stacker++]);
   video::pixel(x, y, color);
   return SENTINEL;
  }

  addr text(fpu) {
   BAIL_UNLESS_STACK_ATLEAST(4)
   s32 color = cast(s32, memory[stacker++]);
   s32 address = cast(s32, memory[stacker++]);
   s32 y = cast(s32, memory[stacker++]);
   s32 x = cast(s32, memory[stacker++]);
   video::text(x, y, utility::string_pick(address).c_str(), color, 0);
   return SENTINEL;
  }

  addr flip(fpu) {
   video::flip();
   return SENTINEL;
  }
 }

 void builtin_register() {
  builtin::add("clear", wrap::clear);
  builtin::add("pixel", wrap::pixel);
  builtin::add("text", wrap::text);
  builtin::add("flip", wrap::flip);
 }
}