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
#include <unordered_set> // IWYU pragma: export
#include <sstream> // IWYU pragma: export
#include <string> // IWYU pragma: export
#include <vector> // IWYU pragma: export

#include "core/fixed_point_unit.hpp" // IWYU pragma: export

using namespace std;

using u8 = uint8_t;
using s8 = int8_t;
using u16 = uint16_t;
using s16 = int16_t;
using u32 = uint32_t;
using s32 = int32_t;
using u64 = uint64_t;
using s64 = int64_t;

using octo = u8; // byte

using address = u32; // vm memory
using address_logic = u16; // per process logic region
using address_local = u16; // per process local region
using opcode_call = address_logic(*)();

using str = const char*;

#define cast(type, value) static_cast<type>(value)
#define reinterpret(type, value) reinterpret_cast<type>(value)

#define hash_map unordered_map
#define hash_set unordered_set