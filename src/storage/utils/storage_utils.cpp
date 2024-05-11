#include "storage_utils.hpp"


namespace cosmo::storage {
    std::optional<fs::path> searchFile(const fs::directory_entry& directory, const std::string& filename) {
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

    std::vector<fs::path> seachFiles(const fs::directory_entry& directory, const std::string& filename) {
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

    std::unique_ptr<char[]> createNullTerminatedCharArr(std::streamsize size)
    {
        auto buffer = std::make_unique<char[]>(size + 1);

        buffer[size] = '\0';

        return buffer;
    }
}