// Compatibility replacement for Android builds.
// kenix3/libultraship Ship::Mobile only has ImGuiProcessEvent.
// This header shadows libultraship/include/ship/port/mobile/MobileImpl.h.
#pragma once

#include <cstdint>
#include <string>

#include <imgui.h>

namespace Ship {

class Mobile {
  public:
    static void ImGuiProcessEvent(bool wantsTextInput);
};

}; // namespace Ship
