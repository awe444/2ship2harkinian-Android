// Compatibility replacement for Android builds.
// kenix3/libultraship (b2dd85ca) Ship::Mobile only has ImGuiProcessEvent,
// but the game's FreeLook.cpp (Android blocks) calls
// IsUsingTouchscreenControls / GetCameraYaw / GetCameraPitch.
// This header shadows libultraship/include/ship/port/mobile/MobileImpl.h
// and provides inline stubs for the missing methods.
#pragma once

#include <cstdint>
#include <string>

#include <imgui.h>

namespace Ship {

class Mobile {
  public:
    static void ImGuiProcessEvent(bool wantsTextInput);

    // Stubs for mobile camera API not yet in kenix3/libultraship.
    static bool IsUsingTouchscreenControls() { return false; }
    static float GetCameraYaw() { return 0.0f; }
    static float GetCameraPitch() { return 0.0f; }
};

}; // namespace Ship
