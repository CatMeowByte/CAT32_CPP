#pragma once

#include <cstdint>

struct fpu {
 static constexpr int DECIMAL_WIDTH = 16;

 int32_t value;

 fpu() = default;
 fpu(bool x) : value(x ? (1 << DECIMAL_WIDTH) : 0) {}
 fpu(double x) {value = static_cast<int32_t>(x * (1 << DECIMAL_WIDTH));}

 fpu(int32_t raw, bool) : value(raw) {}

 fpu& operator++() {value += (1 << DECIMAL_WIDTH); return *this;}
 fpu operator++(int) {fpu temp = *this; value += (1 << DECIMAL_WIDTH); return temp;}

 fpu& operator--() {value -= (1 << DECIMAL_WIDTH); return *this;}
 fpu operator--(int) {fpu temp = *this; value -= (1 << DECIMAL_WIDTH); return temp;}

 fpu operator+(const fpu& other) const {return fpu(value + other.value, true);}
 fpu operator-(const fpu& other) const {return fpu(value - other.value, true);}
 fpu operator*(const fpu& other) const {return fpu(static_cast<int32_t>((static_cast<int64_t>(value) * other.value) >> DECIMAL_WIDTH), true);}
 fpu operator/(const fpu& other) const {return fpu(static_cast<int32_t>((static_cast<int64_t>(value) << DECIMAL_WIDTH) / other.value), true);}

 fpu& operator+=(const fpu& other) {value += other.value; return *this;}
 fpu& operator-=(const fpu& other) {value -= other.value; return *this;}
 fpu& operator*=(const fpu& other) {*this = *this * other; return *this;}
 fpu& operator/=(const fpu& other) {*this = *this / other; return *this;}

 bool operator==(const fpu& other) const {return value == other.value;}
 bool operator!=(const fpu& other) const {return value != other.value;}
 bool operator<(const fpu& other) const {return value < other.value;}
 bool operator<=(const fpu& other) const {return value <= other.value;}
 bool operator>(const fpu& other) const {return value > other.value;}
 bool operator>=(const fpu& other) const {return value >= other.value;}

 fpu operator&(const fpu& other) const {return fpu(value & other.value, true);}
 fpu operator|(const fpu& other) const {return fpu(value | other.value, true);}
 fpu operator~() const {return fpu(~value, true);}
 fpu operator<<(int shift) const {return fpu(value << shift, true);}
 fpu operator>>(int shift) const {return fpu(value >> shift, true);}

 #define FPU_INTEGER_CONVERSIONS \
  X(int8_t) X(uint8_t) \
  X(int16_t) X(uint16_t) \
  X(int32_t) X(uint32_t) \
  X(int64_t) X(uint64_t)

 #define FPU_FLOAT_CONVERSIONS \
  X(float) X(double)

 #define X(TYPE) operator TYPE() const {return static_cast<TYPE>(value >> DECIMAL_WIDTH);}
 FPU_INTEGER_CONVERSIONS
 #undef X

 #define X(TYPE) operator TYPE() const {return static_cast<TYPE>(value) / static_cast<TYPE>(1 << DECIMAL_WIDTH);}
 FPU_FLOAT_CONVERSIONS
 #undef X

 #undef FPU_INTEGER_CONVERSIONS
 #undef FPU_FLOAT_CONVERSIONS

 operator bool() const {return value != 0;}
};