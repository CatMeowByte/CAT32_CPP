#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace kernel {
 enum class Event : u8 {Load = 15, Init = 0, Step = 5, Draw = 10};

 void run_event(Event handler);
}