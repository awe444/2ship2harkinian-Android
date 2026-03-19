// Android touch-controller JNI bridge.
//
// The old Waterdish/libultraship contained these JNI functions in
// src/port/mobile/MobileImpl.cpp.  When the submodule was switched to
// kenix3/libultraship (which doesn't have them), the native
// implementations disappeared and MainActivity.attachController() threw
// UnsatisfiedLinkError at startup.
//
// This file provides:
//   - Ship::Mobile methods used by FreeLook.cpp's #ifdef __ANDROID__ blocks
//   - JNI functions called from the Java touch-control overlay in
//     com.dishii.mm.MainActivity

#ifdef __ANDROID__

#include "port/mobile/MobileImpl.h"

#include <SDL2/SDL.h>
#include <SDL_joystick.h>
#include <jni.h>

// ── shared state ──────────────────────────────────────────────────────
static float cameraYaw   = 0.0f;
static float cameraPitch = 0.0f;
static bool  isUsingTouchscreenControls = false;

static int            virtual_joystick_id = -1;
static SDL_Joystick  *virtual_joystick    = nullptr;

// ── Ship::Mobile method implementations ──────────────────────────────
bool Ship::Mobile::IsUsingTouchscreenControls() {
    return isUsingTouchscreenControls;
}

float Ship::Mobile::GetCameraYaw() {
    return cameraYaw;
}

float Ship::Mobile::GetCameraPitch() {
    return cameraPitch;
}

// ── JNI entry points ─────────────────────────────────────────────────

extern "C" void JNICALL
Java_com_dishii_mm_MainActivity_attachController(JNIEnv* /*env*/, jobject /*obj*/) {
    virtual_joystick_id = SDL_JoystickAttachVirtual(
        SDL_JOYSTICK_TYPE_GAMECONTROLLER, 6, 18, 0);
    if (virtual_joystick_id == -1) {
        SDL_Log("Could not create overlay virtual controller");
        return;
    }

    virtual_joystick = SDL_JoystickOpen(virtual_joystick_id);
    if (virtual_joystick == nullptr)
        SDL_Log("Could not create virtual joystick");

    isUsingTouchscreenControls = true;
}

extern "C" void JNICALL
Java_com_dishii_mm_MainActivity_detachController(JNIEnv* /*env*/, jobject /*obj*/) {
    if (virtual_joystick != nullptr) {
        SDL_JoystickClose(virtual_joystick);
        virtual_joystick = nullptr;
    }
    if (virtual_joystick_id != -1) {
        SDL_JoystickDetachVirtual(virtual_joystick_id);
        virtual_joystick_id = -1;
    }
    isUsingTouchscreenControls = false;
}

extern "C" void JNICALL
Java_com_dishii_mm_MainActivity_setButton(JNIEnv* /*env*/, jobject /*obj*/,
                                          jint button, jboolean value) {
    if (virtual_joystick == nullptr)
        return;

    if (button < 0) {
        // Negative button ids map to axes (e.g. -4 → axis 4 for triggers).
        // Using -SDL_MAX_SINT16 for "released" instead of 0 is intentional:
        // the original Waterdish code noted an SDL bug where 0 didn't register.
        SDL_JoystickSetVirtualAxis(virtual_joystick, -button,
                                   value ? SDL_MAX_SINT16 : -SDL_MAX_SINT16);
    } else {
        SDL_JoystickSetVirtualButton(virtual_joystick, button, value);
    }
}

extern "C" void JNICALL
Java_com_dishii_mm_MainActivity_setAxis(JNIEnv* /*env*/, jobject /*obj*/,
                                        jint axis, jshort value) {
    if (virtual_joystick == nullptr)
        return;
    SDL_JoystickSetVirtualAxis(virtual_joystick, axis, value);
}

extern "C" void JNICALL
Java_com_dishii_mm_MainActivity_setCameraState(JNIEnv* /*env*/, jobject /*obj*/,
                                               jint axis, jfloat value) {
    switch (axis) {
        case 0: cameraYaw   = value; break;
        case 1: cameraPitch = value; break;
    }
}

#endif // __ANDROID__
