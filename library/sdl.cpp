#include "library/sdl.hpp"

// TODO: handle filesystem

static constexpr u32 PALETTE[16] = {
 0xFF000000, 0xFF1D2B53, 0xFF7E2553, 0xFF008751,
 0xFFAB5236, 0xFF5F574F, 0xFFC2C3C7, 0xFFFFF1E8,
 0xFFFF004D, 0xFFFFA300, 0xFFFFEC27, 0xFF00E436,
 0xFF29ADFF, 0xFF83769C, 0xFFFF77A8, 0xFFFFCCAA,
};

//audio
static SDL_AudioStream* stream = nullptr;
static s16 buffer[AUDIO::BUFFER];
static u32 phase[AUDIO::CHANNEL];

static void audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
 if (!additional_amount) {return;}

 u32 sample = min(cast(u32, additional_amount / sizeof(s16)), cast(u32, AUDIO::BUFFER));
 fpu* data = cast(fpu*, userdata);

 for (u32 i = 0; i < sample; i++) {
  buffer[i] = 0;
  for (u8 ch = 0; ch < AUDIO::CHANNEL; ch++) {
   if (!data[ch].value || data[ch].value > cast(s32, (AUDIO::RATE / 2) << fpu::DECIMAL_WIDTH)) {continue;}
   buffer[i] += ((cast(s32, phase[ch] >> fpu::DECIMAL_WIDTH) < data[ch + AUDIO::CHANNEL].value) ? INT16_MAX : INT16_MIN) / AUDIO::CHANNEL;
   phase[ch] += (cast(u64, data[ch].value) << (32 - fpu::DECIMAL_WIDTH)) / AUDIO::RATE;
  }
 }

 SDL_PutAudioStreamData(stream, buffer, sample * sizeof(s16));
}

// video
static SDL_Window* window = nullptr;
static SDL_Surface* surface_front = nullptr; // what shows on screen
static SDL_Surface* surface_back = nullptr;   // 4bpp framebuffer as SDL surface
static SDL_Palette* palette = nullptr;

// button
static bool key_state[SDL_SCANCODE_COUNT];

namespace sdl {
 bool audio(fpu* data) {
  if (!SDL_Init(SDL_INIT_AUDIO)) {return false;}
  static const SDL_AudioSpec spec = {SDL_AUDIO_S16, 1, AUDIO::RATE};
  stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, audio_callback, data);
  if (!stream) {return false;}
  SDL_ResumeAudioStreamDevice(stream);
  return true;
 }

 bool video(octo* data) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {return false;}
  window = SDL_CreateWindow(SYSTEM::NAME, VIDEO::WIDTH * VIDEO::SCALE, VIDEO::HEIGHT * VIDEO::SCALE, SDL_WINDOW_ALWAYS_ON_TOP);
  if (!window) {return false;}
  surface_front = SDL_GetWindowSurface(window);
  if (!surface_front) {return false;}
  surface_back = SDL_CreateSurfaceFrom(VIDEO::WIDTH, VIDEO::HEIGHT, SDL_PIXELFORMAT_INDEX4MSB, data, VIDEO::WIDTH / 2);
  if (!surface_back) {return false;}
  palette = SDL_CreatePalette(16);
  if (!palette) {return false;}

  SDL_Color colors[16];
  for (u8 i = 0; i < 16; i++) {
   u32 c = PALETTE[i];
   colors[i].r = (c >> 16) & 0xFF;
   colors[i].g = (c >> 8) & 0xFF;
   colors[i].b = c & 0xFF;
   colors[i].a = 255;
  }
  SDL_SetPaletteColors(palette, colors, 0, 16);
  SDL_SetSurfacePalette(surface_back, palette);

  // wayland expects something is drawn to show a window
  SDL_ClearSurface(surface_front, 0, 0, 0, 1);
  SDL_UpdateWindowSurface(window);

  return true;
 }

 bool poll() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
   if (e.type == SDL_EVENT_QUIT) {return false;}
   if (e.type == SDL_EVENT_KEY_DOWN) {key_state[e.key.scancode] = true;}
   if (e.type == SDL_EVENT_KEY_UP) {key_state[e.key.scancode] = false;}
  }
  return true;
 }

 void stop() {
  if (stream) {SDL_DestroyAudioStream(stream); stream = nullptr;}
  if (window) {SDL_DestroyWindow(window);}
  if (surface_back) {SDL_DestroySurface(surface_back);}
  if (palette) {SDL_DestroyPalette(palette); palette = nullptr;}
  SDL_Quit();
 }

 void flip() {
  SDL_BlitSurfaceScaled(surface_back, nullptr, surface_front, nullptr, SDL_SCALEMODE_NEAREST);
  SDL_UpdateWindowSurface(window);
 }

 bool is_key_pressed(const str key_name) {
  SDL_Scancode scancode = SDL_GetScancodeFromName(key_name);
  if (scancode == SDL_SCANCODE_UNKNOWN) {return false;}
  return key_state[scancode];
 }
}