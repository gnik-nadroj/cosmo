#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <functional>

struct TemporaryFile {

    TemporaryFile(const std::filesystem::path& directory, const std::filesystem::path& filename) 
        : filePath(directory / filename) 
    {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            throw std::invalid_argument("Failed to create the file at: " + filePath.string());
        }
        file.close();
    }

    TemporaryFile(const TemporaryFile&) = delete;
    TemporaryFile& operator = (const TemporaryFile&) = delete;

     TemporaryFile(TemporaryFile&& other) noexcept 
        : filePath(std::move(other.filePath)) 
    {
        other.filePath.clear();
    }

    TemporaryFile& operator=(TemporaryFile&& other) noexcept {
        if (this != &other) {
            if(!filePath.empty()) {
                std::filesystem::remove(filePath);
            }
            filePath = std::move(other.filePath);
            other.filePath.clear();
        }
        return *this;
    }

    ~TemporaryFile() {
        std::filesystem::remove(filePath);
    }

    std::filesystem::path filePath;
};
