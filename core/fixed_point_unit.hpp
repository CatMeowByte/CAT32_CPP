#pragma once

#include <cstdint>

struct fpu {
 static constexpr uint8_t WIDTH = 16;

private:
 int32_t value;
 constexpr fpu(int32_t v, bool) : value(v) {}

public:
 fpu() = default;
 constexpr fpu(double x) : value(static_cast<int32_t>(x * (1 << WIDTH))) {}

 static constexpr fpu raw(int32_t r) {return fpu(r, true);}

 constexpr uint32_t a() const {return static_cast<uint32_t>(value >> WIDTH);}
 constexpr double d() const {return static_cast<double>(value) / (1 << WIDTH);}
 constexpr int16_t i() const {return static_cast<int16_t>(value >> WIDTH);}
 constexpr int32_t r() const {return value;}

 constexpr operator double() const {return d();}

 fpu& operator++() {value += (1 << WIDTH); return *this;}
 fpu operator++(int) {fpu t = *this; value += (1 << WIDTH); return t;}
 fpu& operator--() {value -= (1 << WIDTH); return *this;}
 fpu operator--(int) {fpu t = *this; value -= (1 << WIDTH); return t;}

 constexpr fpu operator+(fpu o) const {return fpu(value + o.value, true);}
 constexpr fpu operator-(fpu o) const {return fpu(value - o.value, true);}
 constexpr fpu operator*(fpu o) const {return fpu(static_cast<int32_t>((static_cast<int64_t>(value) * o.value) >> WIDTH), true);}
 constexpr fpu operator/(fpu o) const {return fpu(static_cast<int32_t>((static_cast<int64_t>(value) << WIDTH) / o.value), true);}

 fpu& operator+=(fpu o) {value += o.value; return *this;}
 fpu& operator-=(fpu o) {value -= o.value; return *this;}
 fpu& operator*=(fpu o) {*this = *this * o; return *this;}
 fpu& operator/=(fpu o) {*this = *this / o; return *this;}

 constexpr bool operator==(fpu o) const {return value == o.value;}
 constexpr bool operator!=(fpu o) const {return value != o.value;}
 constexpr bool operator<(fpu o) const {return value < o.value;}
 constexpr bool operator<=(fpu o) const {return value <= o.value;}
 constexpr bool operator>(fpu o) const {return value > o.value;}
 constexpr bool operator>=(fpu o) const {return value >= o.value;}

 constexpr fpu operator&(fpu o) const {return fpu(value & o.value, true);}
 constexpr fpu operator|(fpu o) const {return fpu(value | o.value, true);}
 constexpr fpu operator~() const {return fpu(~value, true);}
 constexpr fpu operator<<(int32_t s) const {return fpu(value << s, true);}
 constexpr fpu operator>>(int32_t s) const {return fpu(value >> s, true);}
 constexpr fpu operator-() const {return fpu(-value, true);}
};