#include "storage.hpp"

namespace cosmo::io{
    std::optional<fs::path> Storage::getActiveFilePath() {
        for (const auto& entry : fs::directory_iterator(_storage_directory)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.rfind("active", 0) == 0) {
                    return entry.path();
                }
            }
        }
        return std::nullopt;
    }
};