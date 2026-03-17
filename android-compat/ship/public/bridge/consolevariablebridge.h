// Compatibility shim for Android builds.
// kenix3/libultraship's MobileImpl.cpp (mobile-only, never tested on PC by
// upstream) includes "ship/public/bridge/consolevariablebridge.h", but the
// actual header lives at include/libultraship/bridge/consolevariablebridge.h.
#include "libultraship/bridge/consolevariablebridge.h"
