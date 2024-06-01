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
#include <atomic>

namespace fs = std::filesystem;

namespace cosmo::storage {
    using offset = std::fstream::pos_type;
    using data_file_size = uint32_t;
    using data_file_id = uint32_t;

    using ReadResult = std::pair<bool, char*>;
    using WriteResult = std::tuple<bool, data_file_id, offset>;

    inline static const auto APPEND_READ = std::ios::app | std::ios::in;

    std::optional<fs::path> searchFile(const fs::directory_entry& directory, const std::string_view filename);
    std::vector<fs::path> seachFiles(const fs::directory_entry& directory, const std::string_view filename);

    class CharBuffer {
    private:
        static inline std::mutex _mtx;
        static inline size_t _size{ 1'000'000'000 };
        static inline size_t _current_pos{};
        static inline auto _buffer = std::make_unique<char[]>(_size);

    public:
        static char* getBuffer(std::streamsize requestedSize) {
            std::scoped_lock lck{ _mtx };

            if (requestedSize > _size) {
                throw std::runtime_error("Requested size is larger than buffer size");
            }

            if (_current_pos + requestedSize >= _size) {
                _current_pos = 0;
            }

            char* start = &_buffer[_current_pos];
            _buffer[_current_pos + requestedSize] = '\0';
            _current_pos += requestedSize + 1;

            return start;
        }
    };

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

    class ConcurrentFile {
    public:
        ConcurrentFile() = default;

        ~ConcurrentFile() = default;

        ConcurrentFile(const ConcurrentFile&) = delete;
        ConcurrentFile& operator=(const ConcurrentFile&) = delete;

        ConcurrentFile(const fs::path& filePath, std::ios_base::openmode mode = APPEND_READ)
            : _writer{ filePath, mode }, _file_path{ filePath } {
            if (!_writer.is_open()) {
                throw std::invalid_argument("Unable to in file");
            }
        }

        ConcurrentFile(ConcurrentFile&& other) noexcept {
            std::scoped_lock lock{ _write_mtx, other._write_mtx };
            _file_path = std::move(other._file_path);
            _writer = std::move(other._writer);
        }

        ConcurrentFile& operator=(ConcurrentFile&& other) noexcept {
            if (this != &other) {
                std::scoped_lock lock{ _write_mtx, other._write_mtx };
                _file_path = std::move(other._file_path);
                _writer = std::move(other._writer);
            }
            return *this;
        }

        std::pair<bool, char*> read(std::fstream::pos_type offset, std::streamsize size) const {
            return safeIoOperation([this, &size, &offset] {
                auto buffer = CharBuffer::getBuffer(size);

                std::ifstream reader {_file_path, std::ios::in};

                reader.seekg(offset);

                reader.read(buffer, size);

                return buffer;
            });
        }

        std::pair<bool, std::fstream::pos_type> write(const char* value, std::streamsize size) {
            return safeIoOperation([this, &value, &size] {
                std::scoped_lock lck{ _write_mtx };

                auto pos = _writer.tellp();
                
                _writer.write(value, size);

                return pos;
            });
        }

        bool isOpen() const {
            return _writer.is_open();
        }

        const fs::path& getPath() const {
            return _file_path;
        }

    private:
        std::fstream _writer{};
        std::mutex _write_mtx;
        fs::path _file_path{};
    };
}