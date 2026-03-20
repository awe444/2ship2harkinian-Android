cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
set(GAME_STR "MM")
project(2s2h VERSION 4.0.2 LANGUAGES C CXX)
include(CMake/2ship-cvars.cmake)
include(CMake/lus-cvars.cmake)
set(SPDLOG_LEVEL_TRACE 0)
set(SPDLOG_LEVEL_OFF 6)
set(SPDLOG_MIN_CUTOFF SPDLOG_LEVEL_TRACE CACHE STRING "cutoff at trace")
set(PROJECT_BUILD_NAME "Keiichi Charlie" CACHE STRING "" FORCE)
set(PROJECT_TEAM "github.com/harbourmasters" CACHE STRING "" FORCE)

execute_process(
    COMMAND git branch --show-current
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CMAKE_PROJECT_GIT_BRANCH "${GIT_BRANCH}" CACHE STRING "Git branch" FORCE)

execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get only the first 7 characters of the hash
string(SUBSTRING "${GIT_COMMIT_HASH}" 0 7 SHORT_COMMIT_HASH)

set(CMAKE_PROJECT_GIT_COMMIT_HASH "${SHORT_COMMIT_HASH}" CACHE STRING "Git commit hash" FORCE)

execute_process(
    COMMAND git describe --tags --abbrev=0 --exact-match HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT GIT_COMMIT_TAG)
    set(GIT_COMMIT_TAG "" CACHE STRING "Git commit tag" FORCE)
endif()

set(CMAKE_PROJECT_GIT_COMMIT_TAG "${GIT_COMMIT_TAG}" CACHE STRING "Git commit tag" FORCE)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT 2ship)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if (CMAKE_C_COMPILER_LAUNCHER MATCHES "ccache|sccache")
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT Embedded)
    endif()
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Windows|Linux")
    if(NOT DEFINED BUILD_CROWD_CONTROL)
        set(BUILD_CROWD_CONTROL OFF)
    endif()
endif()

# Enable the Gfx debugger in LUS to use libgfxd from ZAPDTR
set(GFX_DEBUG_DISASSEMBLER ON)

# Tell LUS we're using F3DEX_GBI_2 (in a way that doesn't break libgfxd)
set(GBI_UCODE F3DEX_GBI_2)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

# Enable MPQ and OTR support
set(INCLUDE_MPQ_SUPPORT ON)

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(NOT CMAKE_VS_PLATFORM_NAME)
        set(CMAKE_VS_PLATFORM_NAME "x64")
    endif()
    message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

    if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"
        OR "${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32"))
        message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
    endif()
endif()

################################################################################
# VCPKG dependencies for Windows
################################################################################
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
include(CMake/automate-vcpkg.cmake)

if ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
	set(VCPKG_TRIPLET x86-windows-static)
	set(VCPKG_TARGET_TRIPLET x86-windows-static)
elseif ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
	set(VCPKG_TRIPLET x64-windows-static)
	set(VCPKG_TARGET_TRIPLET x64-windows-static)
endif()

vcpkg_bootstrap()
vcpkg_install_packages(zlib bzip2 libzip libpng sdl2 sdl2-net glew glfw3 nlohmann-json tinyxml2 spdlog libogg libvorbis opus opusfile)
endif()

################################################################################
# Global configuration types
################################################################################
if (CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
    set(CMAKE_C_FLAGS_DEBUG "-g -ffast-math -DDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -ffast-math -DDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "-O3 -ffast-math -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math -DNDEBUG")
elseif(MSVC)
    # We need to remove RTC1 as it is not compatible with O2, which we set for some files in DEBUG
    string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_C_FLAGS_RELEASE "/O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else()
    set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
    set(CMAKE_OBJCXX_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

if(NOT CMAKE_BUILD_TYPE )
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    get_linux_lsb_release_information()
    message(STATUS "Linux ${LSB_RELEASE_ID_SHORT} ${LSB_RELEASE_VERSION_SHORT} ${LSB_RELEASE_CODENAME_SHORT}")
else()
    message(STATUS ${CMAKE_SYSTEM_NAME})
endif()

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include(CMake/GlobalSettingsInclude.cmake OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Android FetchContent dependencies
# libultraship and ZAPDTR use find_package() for several libraries that aren't
# available as system packages on Android.  Declaring them here with
# OVERRIDE_FIND_PACKAGE lets those later find_package() calls succeed.
################################################################################
if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    include(FetchContent)

    # SDL2 must match the Java SDLActivity.java version constants (2.32.8).
    # kenix3/libultraship's android.cmake fetches SDL 2.28.1 which causes a
    # version mismatch (mBrokenLibraries = true) and an immediate NPE crash.
    # Providing SDL 2.32.8 here with OVERRIDE_FIND_PACKAGE makes
    # libultraship's find_package(SDL2) use this version instead.
    find_package(SDL2 QUIET)
    if (NOT ${SDL2_FOUND})
        FetchContent_Declare(
            SDL2
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-2.32.8
            OVERRIDE_FIND_PACKAGE
        )
        FetchContent_MakeAvailable(SDL2)
    endif()

    find_package(nlohmann_json QUIET)
    if (NOT ${nlohmann_json_FOUND})
        FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.11.3
            OVERRIDE_FIND_PACKAGE
        )
        FetchContent_MakeAvailable(nlohmann_json)
    endif()

    find_package(tinyxml2 QUIET)
    if (NOT ${tinyxml2_FOUND})
        set(tinyxml2_BUILD_TESTING OFF)
        FetchContent_Declare(
            tinyxml2
            GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
            GIT_TAG 10.0.0
            OVERRIDE_FIND_PACKAGE
        )
        FetchContent_MakeAvailable(tinyxml2)
    endif()

    find_package(spdlog QUIET)
    if (NOT ${spdlog_FOUND})
        FetchContent_Declare(
            spdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG v1.14.1
            OVERRIDE_FIND_PACKAGE
        )
        FetchContent_MakeAvailable(spdlog)
    endif()

    find_package(libzip QUIET)
    if (NOT ${libzip_FOUND})
        set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
        set(BUILD_TOOLS OFF)
        set(BUILD_REGRESS OFF)
        set(BUILD_EXAMPLES OFF)
        set(BUILD_DOC OFF)
        set(BUILD_OSSFUZZ OFF)
        set(BUILD_SHARED_LIBS OFF)
        FetchContent_Declare(
            libzip
            GIT_REPOSITORY https://github.com/nih-at/libzip.git
            GIT_TAG v1.10.1
            OVERRIDE_FIND_PACKAGE
        )
        FetchContent_MakeAvailable(libzip)
    endif()

    find_package(PNG QUIET)
    if (NOT ${PNG_FOUND})
        set(PNG_SHARED ON CACHE BOOL "" FORCE)
        set(PNG_STATIC OFF CACHE BOOL "" FORCE)
        set(PNG_TESTS OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(
            PNG
            GIT_REPOSITORY https://github.com/glennrp/libpng.git
            GIT_TAG v1.6.40
            OVERRIDE_FIND_PACKAGE
        )
        FetchContent_MakeAvailable(PNG)
        # libpng creates png_shared but not the PNG::PNG imported target that
        # FindPNG.cmake normally provides.  ZAPD links against PNG::PNG.
        # Build as SHARED so libpng16.so is packaged into the APK (matching
        # the old working build).
        if (NOT TARGET PNG::PNG)
            add_library(PNG::PNG ALIAS png_shared)
        endif()
        set(PNG_PNG_INCLUDE_DIR "${png_SOURCE_DIR}" CACHE PATH "" FORCE)
    endif()
endif()

################################################################################
# Android build-time source patches for libultraship
# These patches are applied at configure time before add_subdirectory().
################################################################################
if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    # GLES 3.0 only guarantees GL_RGBA/GL_UNSIGNED_BYTE for glReadPixels.
    # The upstream ReadFramebufferToCPU uses GL_UNSIGNED_SHORT_5_5_5_1 which
    # silently returns zeros on GLES, causing Pictobox photos to be black.
    # Read as RGBA8888 and convert to RGBA5551 in software.
    set(_GFX_OGL_FILE "${CMAKE_SOURCE_DIR}/libultraship/src/fast/backends/gfx_opengl.cpp")
    file(READ "${_GFX_OGL_FILE}" _GFX_OGL_CONTENT)
    string(FIND "${_GFX_OGL_CONTENT}" "USE_OPENGLES // GLES ReadFramebufferToCPU fix" _GLES_PATCHED)
    if (_GLES_PATCHED EQUAL -1)
        string(REPLACE
            "glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, (void*)rgba16_buf);"
            [[
#ifdef USE_OPENGLES // GLES ReadFramebufferToCPU fix
    {
        uint32_t num_pixels = width * height;
        uint8_t* rgba8_buf = new uint8_t[num_pixels * 4];
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgba8_buf);
        for (uint32_t i = 0; i < num_pixels; i++) {
            uint8_t r = rgba8_buf[i * 4 + 0];
            uint8_t g = rgba8_buf[i * 4 + 1];
            uint8_t b = rgba8_buf[i * 4 + 2];
            uint8_t a = rgba8_buf[i * 4 + 3];
            rgba16_buf[i] = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | (a >> 7);
        }
        delete[] rgba8_buf;
    }
#else
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, (void*)rgba16_buf);
#endif]]
            _GFX_OGL_CONTENT "${_GFX_OGL_CONTENT}"
        )
        file(WRITE "${_GFX_OGL_FILE}" "${_GFX_OGL_CONTENT}")
        message(STATUS "Applied GLES ReadFramebufferToCPU patch to libultraship")
    else()
        message(STATUS "GLES ReadFramebufferToCPU patch already applied")
    endif()
endif()

################################################################################
# Sub-projects
################################################################################
add_subdirectory(libultraship ${CMAKE_BINARY_DIR}/libultraship)
target_compile_definitions(libultraship PUBLIC INCLUDE_MPQ_SUPPORT)

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    # The mobile-port sources in kenix3/libultraship use include paths that
    # assume the ship/ subtree is an include root:
    #   MobileImpl.cpp  → #include "MobileImpl.h"
    #   Gui.cpp         → #include "port/mobile/MobileImpl.h"
    # The actual header lives at include/ship/port/mobile/MobileImpl.h, so
    # we need both directories on the search path.
    #
    # MobileImpl.cpp also includes "ship/public/bridge/consolevariablebridge.h"
    # which doesn't exist in the submodule (the real header is at
    # include/libultraship/bridge/consolevariablebridge.h).  The android-compat/
    # directory contains a forwarding shim for this path.
    #
    # android-compat/ must be PUBLIC and listed BEFORE include/ship so that
    # android-compat/port/mobile/MobileImpl.h shadows the submodule's version.
    target_include_directories(libultraship PUBLIC
        ${CMAKE_SOURCE_DIR}/android-compat
        ${CMAKE_SOURCE_DIR}/libultraship/include/ship
    )
    # include/ship/port/mobile is only needed by libultraship's own sources
    # (for bare #include "MobileImpl.h" in MobileImpl.cpp), so it stays PRIVATE.
    target_include_directories(libultraship PRIVATE
        ${CMAKE_SOURCE_DIR}/libultraship/include/ship/port/mobile
    )
    # libultraship's public header O2rArchive.h includes "zip.h".  On PC,
    # libzip is system-installed so zip.h is globally visible.  On Android,
    # it comes from FetchContent and the include dir must be explicitly
    # propagated to all consumers via a PUBLIC link.
    target_link_libraries(libultraship PUBLIC libzip::zip)
endif()
add_subdirectory(ZAPDTR/ZAPD ${CMAKE_BINARY_DIR}/ZAPD)
if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    # ZAPDTR/ZAPD/CrashHandler.cpp includes <execinfo.h> on POSIX platforms,
    # but Android's Bionic libc doesn't provide backtrace() at API < 33.
    # android-compat/execinfo.h supplies inline no-op stubs.  BEFORE ensures
    # it shadows the NDK's header.
    target_include_directories(ZAPDLib BEFORE PRIVATE
        ${CMAKE_SOURCE_DIR}/android-compat
    )
    # libpng generates pnglibconf.h in the build directory, but
    # PNG_PNG_INCLUDE_DIR only points at the source dir.  ZAPD's
    # ImageBackend.cpp includes <png.h> which in turn needs pnglibconf.h.
    target_include_directories(ZAPDLib PRIVATE
        ${png_BINARY_DIR}
    )
endif()
add_subdirectory(OTRExporter)
if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    # OTRExporter includes ZAPDTR/ZAPD headers (e.g. ZResource.h) that
    # transitively need tinyxml2.h.  On PC it's a system header; on Android
    # it comes from FetchContent and must be explicitly linked.
    # OTRExporter also includes ExporterArchiveO2R.h which needs <zip.h>.
    # Although libzip::zip is PUBLIC on libultraship, OTRExporter uses
    # add_dependencies (not target_link_libraries) so the PUBLIC include
    # dirs don't propagate — we must link explicitly here too.
    target_link_libraries(OTRExporter PRIVATE tinyxml2::tinyxml2 libzip::zip)
endif()
add_subdirectory(mm)

set_property(TARGET 2ship PROPERTY APPIMAGE_DESKTOP_FILE_TERMINAL YES)
set_property(TARGET 2ship PROPERTY APPIMAGE_DESKTOP_FILE "${CMAKE_SOURCE_DIR}/mm/linux/2s2h.desktop")
set_property(TARGET 2ship PROPERTY APPIMAGE_ICON_FILE "${CMAKE_BINARY_DIR}/2s2hIcon.png")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
install(FILES "${CMAKE_BINARY_DIR}/mm/2ship.o2r" DESTINATION . COMPONENT ship)
install(TARGETS ZAPD DESTINATION ./assets/extractor COMPONENT extractor)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/mm/assets/extractor/" DESTINATION ./assets/ COMPONENT extractor)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/mm/assets/xml/" DESTINATION ./assets/xml COMPONENT extractor)
endif()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/mm/assets/extractor/" DESTINATION ./assets/ COMPONENT 2s2h)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/mm/assets/xml/" DESTINATION ./assets/xml COMPONENT 2s2h)
endif()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
file(COPY "${CMAKE_SOURCE_DIR}/mm/assets/extractor/" DESTINATION "${CMAKE_SOURCE_DIR}/Android/app/src/main/assets/assets")
file(COPY "${CMAKE_SOURCE_DIR}/mm/assets/xml/" DESTINATION "${CMAKE_SOURCE_DIR}/Android/app/src/main/assets/assets/xml")

if (EXISTS "${CMAKE_SOURCE_DIR}/2ship.o2r")
    file(COPY "${CMAKE_SOURCE_DIR}/2ship.o2r" DESTINATION "${CMAKE_SOURCE_DIR}/Android/app/src/main/assets/")
endif()
endif()


find_package(Python3 COMPONENTS Interpreter)

# Target to generate OTRs
add_custom_target(
    ExtractAssets
    # CMake versions prior to 3.17 do not have the rm command, use remove instead for older versions
    COMMAND ${CMAKE_COMMAND} -E $<IF:$<VERSION_LESS:${CMAKE_VERSION},3.17>,remove,rm> -f mm.zip 2ship.o2r
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/OTRExporter/extract_assets.py -z "$<TARGET_FILE:ZAPD>" --non-interactive --xml-root ../mm/assets/xml --custom-otr-file 2ship.o2r "--custom-assets-path" ${CMAKE_CURRENT_SOURCE_DIR}/mm/assets/custom --port-ver "${CMAKE_PROJECT_VERSION}"
    COMMAND ${CMAKE_COMMAND} -DSYSTEM_NAME=${CMAKE_SYSTEM_NAME} -DTARGET_DIR="$<TARGET_FILE_DIR:ZAPD>" -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DBINARY_DIR=${CMAKE_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/CMake/copy-existing-otrs.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/mm
    COMMENT "Running asset extraction..."
    DEPENDS ZAPD
    BYPRODUCTS mm.o2r ${CMAKE_SOURCE_DIR}/mm.o2r ${CMAKE_SOURCE_DIR}/2ship.o2r
)

# Target to generate headers
add_custom_target(
    ExtractAssetHeaders
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/OTRExporter/extract_assets.py -z "$<TARGET_FILE:ZAPD>" --non-interactive --xml-root ../mm/assets/xml --gen-headers
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/mm
    COMMENT "Generating asset headers..."
    DEPENDS ZAPD
)

# Target to generate only 2ship.o2r
add_custom_target(
    Generate2ShipOtr
    # CMake versions prior to 3.17 do not have the rm command, use remove instead for older versions
    COMMAND ${CMAKE_COMMAND} -E $<IF:$<VERSION_LESS:${CMAKE_VERSION},3.17>,remove,rm> -f 2ship.o2r
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/OTRExporter/extract_assets.py -z "$<TARGET_FILE:ZAPD>" --norom --custom-otr-file 2ship.o2r "--custom-assets-path" ${CMAKE_CURRENT_SOURCE_DIR}/mm/assets/custom --port-ver "${CMAKE_PROJECT_VERSION}"
    COMMAND ${CMAKE_COMMAND} -DSYSTEM_NAME=${CMAKE_SYSTEM_NAME} -DTARGET_DIR="$<TARGET_FILE_DIR:ZAPD>" -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DBINARY_DIR=${CMAKE_BINARY_DIR} -DONLY2SHIPOTR=On -P ${CMAKE_CURRENT_SOURCE_DIR}/CMake/copy-existing-otrs.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/mm
    COMMENT "Generating 2ship.o2r..."
    DEPENDS ZAPD
)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
file(COPY ${CMAKE_SOURCE_DIR}/mm/linux/2s2hIcon.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
add_custom_target(CreateOSXIcons
   COMMAND mkdir -p ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset
   COMMAND sips -z 16 16     mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_16x16.png
   COMMAND sips -z 32 32     mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_16x16@2x.png
   COMMAND sips -z 32 32     mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_32x32.png
   COMMAND sips -z 64 64     mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_32x32@2x.png
   COMMAND sips -z 128 128   mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_128x128.png
   COMMAND sips -z 256 256   mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_128x128@2x.png
   COMMAND sips -z 256 256   mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_256x256.png
   COMMAND sips -z 512 512   mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_256x256@2x.png
   COMMAND sips -z 512 512   mm/macosx/2s2hIcon.png --out ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_512x512.png
   COMMAND cp                mm/macosx/2s2hIcon.png ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset/icon_512x512@2x.png
   COMMAND iconutil -c icns -o ${CMAKE_BINARY_DIR}/macosx/2s2h.icns ${CMAKE_BINARY_DIR}/macosx/2s2h.iconset
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
   COMMENT "Creating OSX icons ..."
   )
add_dependencies(2ship CreateOSXIcons)

install(TARGETS ZAPD DESTINATION ${CMAKE_BINARY_DIR}/assets/extractor)

set(PROGRAM_PERMISSIONS_EXECUTE OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/mm/assets/extractor/" DESTINATION ./assets/)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/mm/assets/xml/" DESTINATION ./assets/xml)

# Rename the installed soh binary to drop the macos suffix
INSTALL(CODE "FILE(RENAME \${CMAKE_INSTALL_PREFIX}/../MacOS/2s2h-macos \${CMAKE_INSTALL_PREFIX}/../MacOS/2s2h)")
install(CODE "
   include(BundleUtilities)
  fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/../MacOS/2s2h\" \"\" \"${dirs}\")
   ")

endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows|NintendoSwitch|CafeOS")
install(FILES ${CMAKE_SOURCE_DIR}/README.md DESTINATION . COMPONENT 2s2h RENAME readme.txt )
install(CODE "file(MAKE_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/mods\")" COMPONENT 2s2h)
install(CODE "file(TOUCH \"\${CMAKE_INSTALL_PREFIX}/mods/custom_mod_files_go_here.txt\")" COMPONENT 2s2h)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(CPACK_GENERATOR "External")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows|NintendoSwitch|CafeOS")
    set(CPACK_GENERATOR "ZIP")
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(CPACK_GENERATOR "Bundle")
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(CPACK_PROJECT_CONFIG_FILE ${CMAKE_SOURCE_DIR}/CMake/Packaging-2.cmake)
    include(CMake/Packaging.cmake)
endif()
