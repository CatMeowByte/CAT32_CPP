#include "CAT32.hpp"
#include "lib/sdl.hpp"

float fps = 0;
u32 lastFrame = sdl_get_ticks();

void init() {
 // user-defined setup logic
 std::ifstream file("/media/beta/share/cpp/CAT32/example/0.app");
 if (!file) {
  std::cerr << "Failed to open file.\n";
 }

 std::string line;
 while (std::getline(file, line)) {
  std::cout << line << "\n";
 }
}

void update() {
 // per-frame logic
}

void draw() {
 clear(1);
 pixel(0, 0, 7);
 pixel(1, 1, 15);

 line(-4, 76, 57, 89, 14);

 rect(-17, 17, 55, 5, 12, true);
 rect(17, 45, 3, 3, 4, false);

 text(0, 0,  " ░▒▓█─│┌┐└┘├┤┬┴┼", 1, 0);
 text(0, 8,  "•°©®™          �", 2, 3);
 text(0, 16, " !\"#$%&'()*+,-./", 3, 2);
 text(0, 24, "0123456789:;<=>?", 4, 1);
 text(0, 32, "@ABCDEFGHIJKLMNO", 5, 0);
 text(0, 40, "PQRSTUVWXYZ[\\]^_", 6, 3);
 text(0, 48, "`abcdefghijklmno", 7, 2);
 text(0, 56, "pqrstuvwxyz{|}~ ", 8, 1);

 u32 now = sdl_get_ticks();
 u32 delta = now - lastFrame;
 lastFrame = now;

 if (delta > 0) {
  fps = 1000.0f / delta;
 }

 char buf[32];
 snprintf(buf, sizeof(buf), "FPS=%.1f", fps);
 text(0, 120, buf, 7, 0);

 flip();
}
