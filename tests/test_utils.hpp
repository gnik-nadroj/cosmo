#include <filesystem>
#include <fstream>
#include <stdexcept>

class TemporaryFile {
    public:
        TemporaryFile(const std::filesystem::path& directory, const std::filesystem::path& filename) 
            : filePath(directory / filename) 
        {
            std::ofstream file(filePath);
            if (!file) {
                throw std::runtime_error("Failed to create the file at: " + filePath.string());
            }
        }

        TemporaryFile(const TemporaryFile&) = delete;
        TemporaryFile& operator = (const TemporaryFile&) = delete;

        ~TemporaryFile() {
            std::filesystem::remove(filePath);
        }

    private:
        std::filesystem::path filePath;
};


