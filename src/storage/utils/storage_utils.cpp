#include "storage_utils.hpp"

#include <string_view>


namespace cosmo::storage {
    std::optional<fs::path> searchFile(const fs::directory_entry& directory, const std::string_view filename) {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string directory_file = entry.path().filename().string();
                if (directory_file == filename) {
                    return entry.path();
                }
            }
        }
        return std::nullopt;
    }

    std::vector<fs::path> seachFiles(const fs::directory_entry& directory, const std::string_view filename) {
        std::vector<fs::path> files{};
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string file = entry.path().filename().string();
                if (file.rfind(filename, 0) == 0) {
                    files.emplace_back(entry.path());
                }
            }
        }

        return files;
    }
}