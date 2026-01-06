#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace string_ops {
 s32 differ(const string& text_a, const string& text_b);
 s32 order(const string& text_a, const string& text_b);
 double to_n(const string& text);
}