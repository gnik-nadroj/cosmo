#pragma once


#include <optional>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace cosmo::io {
    class FileHandle {
        public:
            static const auto APPEND_READ = std::ios::app | std::ios::in;

            FileHandle() = default;

            FileHandle(const fs::path& filePath, std::ios_base::openmode mode = APPEND_READ)
                : _file{filePath, mode}, _filePath{filePath} {
                if (!_file.is_open()) {
                    throw std::runtime_error("Unable to open file");
                }
            }

            ~FileHandle() {
                _file.close();
            }

            FileHandle(FileHandle&& other) noexcept
                : _file(std::move(other._file)) {
                other._file = std::fstream();
            }

            FileHandle& operator=(FileHandle&& other) noexcept {
                if (this != &other) {
                    _file.close();

                    _file = std::move(other._file);

                    other._file = std::fstream();
                }
                return *this;
            }

            std::fstream& stream() {
                return _file;
            }

            std::fstream* operator->() {
                return &_file;
            }

            FileHandle(const FileHandle&) = delete;
            FileHandle& operator=(const FileHandle&) = delete;

        private:
            std::fstream _file;
            fs::path _filePath;
    };


    std::optional<fs::path> searchFile(const fs::directory_entry& directory, const std::string& filename);
    std::vector<fs::path> seachFiles(const fs::directory_entry&  directory, const std::string& filename);
}