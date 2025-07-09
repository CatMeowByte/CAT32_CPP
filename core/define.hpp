#pragma once

#include <algorithm> // IWYU pragma: export
#include <cmath> // IWYU pragma: export
#include <cstdint> // IWYU pragma: export
#include <cstdlib> // IWYU pragma: export
#include <cstring> // IWYU pragma: export
#include <fstream> // IWYU pragma: export
#include <iomanip> // IWYU pragma: export
#include <iostream> // IWYU pragma: export
#include <unordered_map> // IWYU pragma: export
#include <sstream> // IWYU pragma: export
#include <string> // IWYU pragma: export
#include <vector> // IWYU pragma: export

using namespace std;

using u8 = uint8_t;
using s8 = int8_t;
using u16 = uint16_t;
using s16 = int16_t;
using u32 = uint32_t;
using s32 = int32_t;
using u64 = uint64_t;
using s64 = int64_t;

using addr = u32;
using elem = s32;

using str = const char*;

#define cast(type, value) static_cast<type>(value)
#define hash_map unordered_map