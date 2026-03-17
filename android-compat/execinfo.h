// Stub execinfo.h for Android builds.
// Android's Bionic libc added <execinfo.h> in API 33, but the functions
// backtrace() and backtrace_symbols() are not available at API 24.
// ZAPDTR/ZAPD/CrashHandler.cpp includes <execinfo.h> on any POSIX platform;
// this shim provides no-op stubs so the (already-disabled) crash handler
// compiles without error.
#pragma once
#include <stddef.h>

static inline int backtrace(void** buffer, int size) {
    (void)buffer;
    (void)size;
    return 0;
}

static inline char** backtrace_symbols(void* const* buffer, int size) {
    (void)buffer;
    (void)size;
    return NULL;
}
