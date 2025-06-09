#include "CAT32.hpp"
#include "lib/sdl.hpp"

void init() {
 // user-defined setup logic
 ifstream file("/media/storage/share/cpp/CAT32/example/0.app");
 if (!file) {
  cerr << "Failed to open file.\n";
 }

 string line;
 while (getline(file, line)) {
  cout << line << "\n";
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

 fps();

 flip();
}
