#include "library/sdl.hpp"

// TODO: handle filesystem

// Sweetie 16
static constexpr u32 PALETTE[16] = {
 0xFF1A1C2C, 0xFF5D275D, 0xFFB13E53, 0xFFEF7D57,
 0xFFFFCD75, 0xFFA7F070, 0xFF38B764, 0xFF257179,
 0xFF29366F, 0xFF3B5DC9, 0xFF41A6F6, 0xFF73EFF7,
 0xFFF4F4F4, 0xFF94B0C2, 0xFF566C86, 0xFF333C57,
};

// audio
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
   if (!data[ch].r() || data[ch].r() > cast(s32, (AUDIO::RATE / 2) << fpu::WIDTH)) {continue;}
   buffer[i] += ((cast(s32, phase[ch] >> fpu::WIDTH) < data[ch + AUDIO::CHANNEL].r()) ? INT16_MAX : INT16_MIN) / AUDIO::CHANNEL;
   phase[ch] += (cast(u64, data[ch].r()) << (32 - fpu::WIDTH)) / AUDIO::RATE;
  }
 }

 SDL_PutAudioStreamData(stream, buffer, sample * sizeof(s16));
}

// button
static SDL_Gamepad* gamepad_data = nullptr;
static fpu* button_data = nullptr;
static u16 button_code[BUTTON::COUNT][BUTTON::COUNT_KEY + 1];
static bool button_state[BUTTON::COUNT];

// video
static SDL_Window* window = nullptr;
static SDL_Surface* surface_back = nullptr;   // 4bpp framebuffer as SDL surface
static SDL_Palette* palette = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_Texture* texture = nullptr;
static SDL_Surface* surface_rgba = nullptr;

static SDL_HitTestResult hit_test(SDL_Window* win, const SDL_Point* area, void* data) {
 return SDL_HITTEST_DRAGGABLE;
}

// camera
static float camera_rotation[3] = {0.25, -0.35, 0.15};
static float camera_depth = 1.5;

// gyroscope
static constexpr float gyroscope_delta = 1.0 / TICK::UPDATE;
static constexpr float gyroscope_stick_scale = 1.5707963 / 32768;
static constexpr float gyroscope_stiffness = 300;
static constexpr float gyroscope_damping = 30;
static float gyroscope_angle[3];
static float gyroscope_speed[3];

// mesh
static constexpr u8 mesh_scale = 2;
static constexpr u8 mesh_grid = 8;
static constexpr u16 mesh_vertex_count = (mesh_grid + 1) * (mesh_grid + 1);
static constexpr u16 mesh_index_count = mesh_grid * mesh_grid * 6;
static SDL_Vertex mesh_vertex[mesh_vertex_count];
static int mesh_index[mesh_index_count];

// window
static constexpr u8 window_scale = mesh_scale + 1;
static constexpr u16 window_size = VIDEO::HEIGHT * window_scale;

namespace sdl {
 // initializer
 bool audio(fpu* data) {
  if (!SDL_Init(SDL_INIT_AUDIO)) {return false;}
  static const SDL_AudioSpec spec = {SDL_AUDIO_S16, 1, AUDIO::RATE};
  stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, audio_callback, data);
  if (!stream) {return false;}
  SDL_ResumeAudioStreamDevice(stream);
  return true;
 }

 bool button(fpu* data) {
  if (!SDL_Init(SDL_INIT_GAMEPAD)) {return false;}
  button_data = data;

  for (u8 i = 0; i < BUTTON::COUNT; i++) {
   for (u8 j = 0; j < BUTTON::COUNT_KEY; j++) {button_code[i][j] = SDL_GetScancodeFromName(BUTTON::KEY[i][j]);}
   button_code[i][BUTTON::COUNT_KEY] = SDL_SCANCODE_COUNT + SDL_GetGamepadButtonFromString(BUTTON::PAD[i]);
  }

  return true;
 }

 bool video(octo* data) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {return false;}

  // window and renderer
  window = SDL_CreateWindow(SYSTEM::NAME, window_size, window_size, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS | SDL_WINDOW_TRANSPARENT);
  if (!window) {return false;}
  SDL_SetWindowHitTest(window, hit_test, nullptr);
  renderer = SDL_CreateRenderer(window, nullptr);
  if (!renderer) {return false;}

  // framebuffer and palette
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

  // texture
  surface_rgba = SDL_CreateSurface(VIDEO::WIDTH, VIDEO::HEIGHT, SDL_PIXELFORMAT_RGBA32);
  if (!surface_rgba) {return false;}
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, VIDEO::WIDTH, VIDEO::HEIGHT);
  if (!texture) {return false;}
  SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

  // mesh index
  for (u8 y = 0; y < mesh_grid; y++) {
   for (u8 x = 0; x < mesh_grid; x++) {
    u16 corner = y * (mesh_grid + 1) + x;
    u16 offset = (y * mesh_grid + x) * 6;
    mesh_index[offset + 0] = corner;
    mesh_index[offset + 1] = corner + 1;
    mesh_index[offset + 2] = corner + mesh_grid + 2;
    mesh_index[offset + 3] = corner;
    mesh_index[offset + 4] = corner + mesh_grid + 2;
    mesh_index[offset + 5] = corner + mesh_grid + 1;
   }
  }

  // wayland expects something is drawn to show a window
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);
  return true;
 }

 // updater
 namespace update {
  void button() {
   for (u8 i = 0; i < BUTTON::COUNT; i++) {
    fpu& slot = button_data[i];
    if (button_state[i]) {slot++;}
    else if (slot > fpu(0)) {slot = fpu(-1);}
    else if (slot == fpu(-1)) {slot = fpu(0);}
   }
  }

  void gyroscope(fpu* data) {
   float device[3] = {};
   if (gamepad_data) {
    device[0] = -SDL_GetGamepadAxis(gamepad_data, SDL_GAMEPAD_AXIS_LEFTY) * gyroscope_stick_scale;
    device[1] = SDL_GetGamepadAxis(gamepad_data, SDL_GAMEPAD_AXIS_LEFTX) * gyroscope_stick_scale;
    device[2] = SDL_GetGamepadAxis(gamepad_data, SDL_GAMEPAD_AXIS_RIGHTX) * gyroscope_stick_scale;
   }

   for (u8 i = 0; i < 3; i++) {
    gyroscope_speed[i] += (gyroscope_stiffness * (device[i] - gyroscope_angle[i]) - gyroscope_damping * gyroscope_speed[i]) * gyroscope_delta;
    gyroscope_angle[i] += gyroscope_speed[i] * gyroscope_delta;
    camera_rotation[i] = gyroscope_angle[i];

    data[i] = gyroscope_speed[i];
   }
  }

  void window() {
   // rotation matrix
   float cos_x = SDL_cosf(camera_rotation[0]), sin_x = SDL_sinf(camera_rotation[0]);
   float cos_y = SDL_cosf(camera_rotation[1]), sin_y = SDL_sinf(camera_rotation[1]);
   float cos_z = SDL_cosf(camera_rotation[2]), sin_z = SDL_sinf(camera_rotation[2]);
   float rotation[6];
   rotation[0] = cos_y * cos_z;
   rotation[1] = -cos_y * sin_z;
   rotation[2] = sin_x * sin_y * cos_z + cos_x * sin_z;
   rotation[3] = cos_x * cos_z - sin_x * sin_y * sin_z;
   rotation[4] = sin_x * sin_z - cos_x * sin_y * cos_z;
   rotation[5] = sin_x * cos_z + cos_x * sin_y * sin_z;

   // projection
   float plane_width = VIDEO::WIDTH * mesh_scale;
   float plane_height = VIDEO::HEIGHT * mesh_scale;
   float focal_length = plane_height * camera_depth;
   float window_center_x = window_size * 0.5;
   float window_center_y = window_size * 0.5;

   // mesh position
   for (u8 grid_y = 0; grid_y <= mesh_grid; grid_y++) {
    for (u8 grid_x = 0; grid_x <= mesh_grid; grid_x++) {
     float u = cast(float, grid_x) / mesh_grid;
     float v = cast(float, grid_y) / mesh_grid;
     float plane_x = (u - 0.5) * plane_width / plane_height;
     float plane_y = v - 0.5;
     float world_x = rotation[0] * plane_x + rotation[1] * plane_y;
     float world_y = rotation[2] * plane_x + rotation[3] * plane_y;
     float world_z = rotation[4] * plane_x + rotation[5] * plane_y;
     float depth = camera_depth - world_z;
     u16 vertex_index = grid_y * (mesh_grid + 1) + grid_x;
     mesh_vertex[vertex_index].position.x = focal_length * world_x / depth + window_center_x;
     mesh_vertex[vertex_index].position.y = focal_length * world_y / depth + window_center_y;
     mesh_vertex[vertex_index].color.r = 1.0;
     mesh_vertex[vertex_index].color.g = 1.0;
     mesh_vertex[vertex_index].color.b = 1.0;
     mesh_vertex[vertex_index].color.a = 1.0;
     mesh_vertex[vertex_index].tex_coord.x = u;
     mesh_vertex[vertex_index].tex_coord.y = v;
    }
   }

   // draw
   SDL_RenderClear(renderer);
   SDL_RenderGeometry(renderer, texture, mesh_vertex, mesh_vertex_count, mesh_index, mesh_index_count);
   SDL_RenderPresent(renderer);
  }
 }

 bool poll() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
   u16 keycode = 0;
   bool state = false;

   switch (e.type) {
    case SDL_EVENT_QUIT: {return false;}

    case SDL_EVENT_GAMEPAD_ADDED: {gamepad_data = SDL_OpenGamepad(e.gdevice.which); break;}
    case SDL_EVENT_GAMEPAD_REMOVED: {SDL_CloseGamepad(gamepad_data); gamepad_data = nullptr; break;}

    case SDL_EVENT_KEY_DOWN: {state = true;}
    case SDL_EVENT_KEY_UP: {keycode = e.key.scancode; break;}

    case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {state = true;}
    case SDL_EVENT_GAMEPAD_BUTTON_UP: {keycode = SDL_SCANCODE_COUNT + e.gbutton.button; break;}

    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
     if (e.button.button == SDL_BUTTON_RIGHT) {return false;}
     break;
    }
   }

   for (u8 i = 0; keycode && i < BUTTON::COUNT; i++) {
    for (u8 j = 0; j < BUTTON::COUNT_KEY + 1; j++) {
     if (button_code[i][j] == keycode) {button_state[i] = state; break;}
    }
   }
  }
  return true;
 }

 void stop() {
  if (stream) {SDL_DestroyAudioStream(stream); stream = nullptr;}
  if (texture) {SDL_DestroyTexture(texture); texture = nullptr;}
  if (surface_rgba) {SDL_DestroySurface(surface_rgba); surface_rgba = nullptr;}
  if (renderer) {SDL_DestroyRenderer(renderer); renderer = nullptr;}
  if (window) {SDL_DestroyWindow(window);}
  if (surface_back) {SDL_DestroySurface(surface_back);}
  if (palette) {SDL_DestroyPalette(palette); palette = nullptr;}
  SDL_Quit();
 }

 void flip() {
  // texture refresh
  SDL_BlitSurface(surface_back, nullptr, surface_rgba, nullptr);
  SDL_UpdateTexture(texture, nullptr, surface_rgba->pixels, surface_rgba->pitch);
 }
}