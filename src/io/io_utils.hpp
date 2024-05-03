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
                : _file{filePath, mode}, _file_path{filePath} {
                if (!_file.is_open()) {
                    throw std::runtime_error("Unable to open file");
                }
            }

            ~FileHandle() {
                _file.close();
            }

            FileHandle(FileHandle&& other) noexcept
                : _file{ std::move(other._file) }, _file_path{ std::move(other._file_path) } {
                other._file = std::fstream();
            }

            FileHandle& operator=(FileHandle&& other) noexcept {
                if (this != &other) {
                    if (_file.is_open()) {
                        _file.close();
                    }

                    _file_path = std::move(other._file_path);
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

            std::fstream& operator *() {
                return _file;
            }

            const fs::path& getPath() const {
                return _file_path;
            }

            FileHandle(const FileHandle&) = delete;
            FileHandle& operator=(const FileHandle&) = delete;

        private:
            std::fstream _file;
            fs::path _file_path;
    };


    std::optional<fs::path> searchFile(const fs::directory_entry& directory, const std::string& filename);
    std::vector<fs::path> seachFiles(const fs::directory_entry&  directory, const std::string& filename);
}