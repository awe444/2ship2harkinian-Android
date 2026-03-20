#include "Spoiler.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include <filesystem>
#include "BenPort.h"

#include <libultraship/libultra/types.h>

std::vector<std::string> Rando::Spoiler::spoilerOptions;
// Lazy-initialized to avoid calling SDL_AndroidGetExternalStoragePath during
// static construction (before SDL JNI is set up on Android).
static const std::filesystem::path& getRandomizerFolderPath() {
    static const std::filesystem::path path(
        Ship::Context::GetPathRelativeToAppDirectory("randomizer", appShortName));
    return path;
}

// This function refreshes the list of spoiler files in the randomizer folder, this list is used in the Randomizer UI,
// and also includes an option to generate a new seed at the top of the list.
void Rando::Spoiler::RefreshOptions() {
    Rando::Spoiler::spoilerOptions.clear();

    Rando::Spoiler::spoilerOptions.push_back("Generate New Seed");
    s32 spoilerFileIndex = -1;

    // ensure the randomizer folder exists
    if (!std::filesystem::exists(getRandomizerFolderPath())) {
        std::filesystem::create_directory(getRandomizerFolderPath());
    }

    // Add all files in the randomizer folder to the list of spoiler options
    for (const auto& entry : std::filesystem::directory_iterator(getRandomizerFolderPath())) {
        if (entry.is_regular_file()) {
            std::string fileName = entry.path().filename().string();
            Rando::Spoiler::spoilerOptions.push_back(fileName);

            // Check if the current file is the one set in the cvar
            if (fileName == CVarGetString("gRando.SpoilerFile", "")) {
                spoilerFileIndex = Rando::Spoiler::spoilerOptions.size() - 1;
            }
        }
    }

    // If the current spoiler file is not in the randomizer folder, reset the cvar
    if (spoilerFileIndex == -1) {
        CVarSetInteger("gRando.SpoilerFileIndex", 0);
        CVarSetString("gRando.SpoilerFile", "");
    } else {
        CVarSetInteger("gRando.SpoilerFileIndex", spoilerFileIndex);
    }
}
