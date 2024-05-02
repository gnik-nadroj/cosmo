#pragma once


#include <optional>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace cosmo::io {
    struct FileStreamDeleter {
        void operator()(std::fstream* ptr) const {
            if(ptr) {
                ptr->close();
                delete ptr;
            }
        }
    };

    std::optional<fs::path> searchFile(const fs::directory_entry& directory, const std::string& filename);
    std::vector<fs::path> seachFiles(const fs::directory_entry&  directory, const std::string& filename);
}