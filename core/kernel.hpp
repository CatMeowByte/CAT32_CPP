#pragma once

#include "core/constant.hpp" // IWYU pragma: keep

namespace kernel {
 enum class Event : u8 {Load = 9, Init = 0, Step = 3, Draw = 6};

 void run_event(Event handler);
}