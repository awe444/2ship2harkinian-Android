// Compatibility replacement for Android builds.
// kenix3/libultraship Ship::Mobile only has ImGuiProcessEvent,
// but the game's FreeLook.cpp (Android blocks) calls
// IsUsingTouchscreenControls / GetCameraYaw / GetCameraPitch.
// This header shadows libultraship/include/ship/port/mobile/MobileImpl.h
// and declares the extra methods.  Definitions live in
// mm/2s2h/android_jni_controller.cpp (compiled into the 2ship target).
#pragma once

#include <cstdint>
#include <string>

#include <imgui.h>

namespace Ship {

class Mobile {
  public:
    static void ImGuiProcessEvent(bool wantsTextInput);

    // Implemented in mm/2s2h/android_jni_controller.cpp together with the
    // JNI touch-controller functions (attachController, setButton, etc.).
    static bool IsUsingTouchscreenControls();
    static float GetCameraYaw();
    static float GetCameraPitch();
};

}; // namespace Ship
