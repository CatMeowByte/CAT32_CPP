#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>

#include "lib/sdl.hpp"

static constexpr u32 PALETTE[16] = {
 0xFF000000, 0xFF1D2B53, 0xFF7E2553, 0xFF008751,
 0xFFAB5236, 0xFF5F574F, 0xFFC2C3C7, 0xFFFFF1E8,
 0xFFFF004D, 0xFFFFA300, 0xFFFFEC27, 0xFF00E436,
 0xFF29ADFF, 0xFF83769C, 0xFFFF77A8, 0xFFFFCCAA,
};

static SDL_Window* window = nullptr;
static SDL_Surface* surface_front = nullptr; // what shows on screen
static SDL_Surface* surface_back = nullptr;   // 4bpp framebuffer as SDL surface
static SDL_Palette* palette = nullptr;
static bool running = true;

namespace sdl {
 bool init() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {return false;}
  window = SDL_CreateWindow(SYSTEM::NAME, VIDEO::WIDTH * VIDEO::SCALE, VIDEO::HEIGHT * VIDEO::SCALE, SDL_WINDOW_ALWAYS_ON_TOP);
  if (!window) {return false;}
  surface_front = SDL_GetWindowSurface(window);
  if (!surface_front) {return false;}
  surface_back = SDL_CreateSurface(VIDEO::WIDTH, VIDEO::HEIGHT, SDL_PIXELFORMAT_INDEX4MSB);
  if (!surface_back) {return false;}
  palette = SDL_CreatePalette(16);
  if (!palette) {return false;}

  SDL_Color colors[16];
  for (int i = 0; i < 16; i++) {
   u32 c = PALETTE[i];
   colors[i].r = (c >> 16) & 0xFF;
   colors[i].g = (c >> 8) & 0xFF;
   colors[i].b = c & 0xFF;
   colors[i].a = 255;
  }
  SDL_SetPaletteColors(palette, colors, 0, 16);
  SDL_SetSurfacePalette(surface_back, palette);

  return true;
 }

 void shutdown() {
  if (window) {SDL_DestroyWindow(window);}
  if (surface_back) {SDL_DestroySurface(surface_back);}
  if (palette) {
   SDL_DestroyPalette(palette);
   palette = nullptr;
  }
  SDL_Quit();
 }

 bool poll() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
   if (e.type == SDL_EVENT_QUIT) {running = false;}
  }
  return running;
 }

 void delay(int ms) {
  SDL_Delay(ms);
 }

 u32 get_ticks() {
  return SDL_GetTicks();
 }

 void flip(const u8* data) {
  memcpy(surface_back->pixels, data, VIDEO::WIDTH * VIDEO::HEIGHT / 2);
  SDL_BlitSurfaceScaled(surface_back, nullptr, surface_front, nullptr, SDL_SCALEMODE_NEAREST);
  SDL_UpdateWindowSurface(window);
 }
}