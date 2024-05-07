#pragma once


#include <optional>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <iostream>
#include <shared_mutex>
#include <functional>

namespace fs = std::filesystem;

namespace cosmo::io {
    using offset = std::fstream::pos_type;
    using data_file_size = uint32_t;
    using data_file_id = uint32_t;

    using ReadResult = std::pair<bool, char*>;
    using WriteResult = std::tuple<bool, data_file_id, offset>;

    inline static const auto APPEND_READ = std::ios::app | std::ios::in;

    template <typename Func, typename ReturnType = std::invoke_result_t<Func>>
    std::pair<bool, ReturnType> safeIoOperation(Func func) {
        try {
            return { true, func() };
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << '\n';
        }
        catch (const std::ios_base::failure& e) {
            std::cerr << "I/O error: " << e.what() << '\n';
        }
        catch (const std::bad_alloc& e) {
            std::cerr << "Memory allocation error: " << e.what() << '\n';
        }
        catch (const std::exception& e) {
            std::cerr << "General error: " << e.what() << '\n';
        }
        return { false, ReturnType{} };
    }

    std::optional<fs::path> searchFile(const fs::directory_entry& directory, const std::string& filename);
    std::vector<fs::path> seachFiles(const fs::directory_entry& directory, const std::string& filename);
    std::unique_ptr<char[]> createNullTerminatedCharArr(std::streamsize size);

    class ConcurrentFile {
    public:
        ConcurrentFile() = default;

        ~ConcurrentFile() = default;

        ConcurrentFile(const fs::path& filePath, std::ios_base::openmode mode = APPEND_READ)
            : _file{ filePath, mode }, _file_path{ filePath } {
            if (!_file.is_open()) {
                throw std::invalid_argument("Unable to in file");
            }
        }

        bool isOpen() const {
            return _file.is_open();
        }

        std::pair<bool, char*> read(std::fstream::pos_type offset, std::streamsize size) {
            return safeIoOperation([this, &size, &offset] {
                auto buffer = createNullTerminatedCharArr(size); //TODO: Remove allocation, performance and memory issue
                std::scoped_lock lck{ _mtx };

                if (!_file.is_open()) {
                    throw std::ios::failure("the file have been closed by another thread");
                }

                _file.seekg(offset);

                _file.read(buffer.get(), size);

                return buffer.release();
                });
        }

        const fs::path& getPath() const {
            return _file_path;
        }

        std::pair<bool, std::fstream::pos_type> write(const char* value, std::streamsize size) {
            return safeIoOperation([this, &value, &size] {
                std::scoped_lock lck{ _mtx };

                if (!_file.is_open()) {
                    throw std::ios::failure("the file have been closed by another thread");
                }

                auto pos = _file.tellp();
                
                _file.write(value, size);

                return pos;
                });
        }

        ConcurrentFile(const ConcurrentFile&) = delete;
        ConcurrentFile& operator=(const ConcurrentFile&) = delete;

        ConcurrentFile(ConcurrentFile&& other) noexcept {
            std::scoped_lock lock{ _mtx, other._mtx};
            _file_path = std::move(other._file_path);
            _file = std::move(other._file);
        }

        ConcurrentFile& operator=(ConcurrentFile&& other) noexcept {
            if (this != &other) {
                std::scoped_lock lock{ _mtx, other._mtx };
                _file_path = std::move(other._file_path);
                _file = std::move(other._file);
            }
            return *this;
        }


    private:
        std::fstream _file{};
        std::mutex _mtx;
        fs::path _file_path{};
    };
}