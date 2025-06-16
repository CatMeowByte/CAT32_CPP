#include "module/font.hpp"
#include "module/video.hpp"
#include "library/sdl.hpp"

static u8 buffer[VIDEO::WIDTH * VIDEO::HEIGHT >> 1];

static s16 cam_x = 0;
static s16 cam_y = 0;
static bool text_wrap = false;

namespace video {
 u8 pixel(s16 x, s16 y) {
  if (x < 0 || y < 0 || x >= VIDEO::WIDTH || y >= VIDEO::HEIGHT) {return 0;}
  u16 index = y * VIDEO::WIDTH + x;
  u16 byte_pos = index / 2;
  u8 shift = (index % 2 == 0) ? 4 : 0;
  return (buffer[byte_pos] >> shift) & 0xF;
 }

 void pixel(s16 x, s16 y, u8 color) {
  if (x < 0 || y < 0 || x >= VIDEO::WIDTH || y >= VIDEO::HEIGHT) {return;}
  u16 index = y * VIDEO::WIDTH + x;
  u16 byte_pos = index / 2;
  u8 shift = (index % 2 == 0) ? 4 : 0;
  buffer[byte_pos] &= ~(0xF << shift);
  buffer[byte_pos] |= (color & 0xF) << shift;
 }

 void lineh(s16 x, s16 y, s16 length, u8 color) {
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

 void line(s16 ax, s16 ay, s16 bx, s16 by, u8 color) {
  if (ay == by) {
   lineh(ax, ay, bx - ax + (bx >= ax ? 1 : -1), color);
   return;
  }

  s16 dx = abs(bx - ax);
  s16 dy = abs(by - ay);
  s16 sx = (ax < bx) ? 1 : -1;
  s16 sy = (ay < by) ? 1 : -1;
  s16 err = (dx > dy ? dx : -dy) / 2;
  s16 e2;

  while (true) {
   pixel(ax, ay, color);
   if (ax == bx && ay == by) {break;}
   e2 = err;
   if (e2 > -dx) {err -= dy; ax += sx;}
   if (e2 < dy) {err += dx; ay += sy;}
  }
 }

 void rect(s16 x, s16 y, s16 width, s16 height, u8 color, bool fill) {
  if (width <= 0 || height <= 0) {return;}
  if (x + width <= 0 || y + height <= 0 || x >= VIDEO::WIDTH || y >= VIDEO::HEIGHT) {return;}

  if (x < 0) {width += x; x = 0;}
  if (y < 0) {height += y; y = 0;}
  if (x + width > VIDEO::WIDTH) {width = VIDEO::WIDTH - x;}
  if (y + height > VIDEO::HEIGHT) {height = VIDEO::HEIGHT - y;}
  if (width <= 0 || height <= 0) {return;}

  if (fill) {
   for (s16 i = 0; i < height; i++) {
    lineh(x, y + i, width, color);
   }
  } else {
   lineh(x, y, width, color);
   if (height > 1) {
    lineh(x, y + height - 1, width, color);
    for (s16 i = 1; i < height - 1; i++) {
     pixel(x, y + i, color);
     if (width > 1) {
      pixel(x + width - 1, y + i, color);
     }
    }
   }
  }
 }

 void text(s16 x, s16 y, str text, u8 color, u8 background) {
  x -= cam_x;
  y -= cam_y;

  s16 current_x = x;
  s16 current_y = y;

  str s = text;
  while (*s) {
   u32 ordinal = utf8_ordinal(s);

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

   for (s16 py = 0; py < FONT::HEIGHT; py++) {
    for (s16 px = 0; px < FONT::WIDTH; px++) {
     bool on = (bits >> (py * FONT::WIDTH + px)) & 1;
     s16 tx = current_x + px;
     s16 ty = current_y + py;
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
}