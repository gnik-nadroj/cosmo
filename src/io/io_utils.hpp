#pragma once


#include <optional>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <iostream>

namespace fs = std::filesystem;

namespace cosmo::io {
    using offset = std::fstream::pos_type;
    using data_file_size = uint32_t;
    using data_file_id = uint8_t;

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


    class UniqueFile {
    public:
        UniqueFile() = default;

        UniqueFile(const fs::path& filePath, std::ios_base::openmode mode = APPEND_READ)
            : _file{ filePath, mode } {
            if (!_file.is_open()) {
                throw std::invalid_argument("Unable to in file");
            }
        }

        ~UniqueFile() {
            _file.close();
        }

        bool isOpen() const {
            return _file.is_open();
        }

        std::pair<bool, char*> read(std::fstream::pos_type offset, std::streamsize size) {
            return safeIoOperation([&] {
                auto buffer = createNullTerminatedCharArr(size);
                std::scoped_lock lck{ _mtx };

                if (!_file.is_open()) {
                    throw std::ios::failure("the file have been closed by another thread");
                }

                _file.seekg(offset);
                _file.read(buffer.get(), size); //potential bottleneck

                return buffer.release();
                });
        }

        std::pair<bool, std::fstream::pos_type> write(const char* value, std::streamsize size) {
            return safeIoOperation([&] {
                std::scoped_lock lck{ _mtx };

                if (!_file.is_open()) {
                    throw std::ios::failure("the file have been closed by another thread");
                }

                auto pos = _file.tellp();
                _file.write(value, size);  //potential bottleneck

                return pos;
                });
        }

        UniqueFile(const UniqueFile&) = delete;
        UniqueFile& operator=(const UniqueFile&) = delete;

        UniqueFile(UniqueFile&& other) noexcept {
            _file = std::move(other._file);
        }

        UniqueFile& operator=(UniqueFile&& other) noexcept {
            if (this != &other) {
                std::scoped_lock lock(_mtx, other._mtx);
                _file = std::move(other._file);
            }
            return *this;
        }

    private:
        std::mutex _mtx;
        std::fstream _file;
    };

   /* class SharedFile {
    public:
        SharedFile(const fs::path& file_path, std::ios_base::openmode mode = APPEND_READ) : _file_path{ file_path } {
            std::unique_lock lck{ _mtx };
            auto& data = _files[_file_path];
            lck.unlock();

            std::call_once(data.initFlag, [&] {
                data.file = std::make_shared<UniqueFile>(_file_path, mode);
                });
            _file = data.file;
        }

        SharedFile(const SharedFile&) = default;
        SharedFile& operator=(const SharedFile&) = default;

        SharedFile(SharedFile&&) = default;
        SharedFile& operator = (SharedFile&&) = default;

        const fs::path& getPath() const {
            return _file_path;
        }

        std::shared_ptr<UniqueFile> operator->() {
            return _file;
        }

    private:
        struct FileData {
            std::once_flag initFlag;
            std::shared_ptr<UniqueFile> file;
        };

        fs::path _file_path{};
        std::shared_ptr<UniqueFile> _file;
        inline static std::unordered_map<fs::path, FileData> _files{}; // SHOULD MAYBE REPLACED BY A THREAD SAFE MAP TO AVOID LOCKING AT THE CLASS LEVEL
        inline static std::mutex _mtx;
    }; */


    //TODO: make the class thread safe
    class SharedFile {

    public:
        SharedFile(const fs::path& file_path, std::ios_base::openmode mode = APPEND_READ) : _file_path{ file_path } {
            if (!_files.contains(_file_path)) {
                UniqueFile f{ _file_path, mode };
                _files.try_emplace(_file_path, std::move(f));
            }
        }

        ~SharedFile() {
            _files.erase(_file_path);
        }

        SharedFile(const SharedFile&) = default;
        SharedFile& operator=(const SharedFile&) = default;

        SharedFile(SharedFile&&) = default;
        SharedFile& operator = (SharedFile&&) = default;

        const fs::path& getPath() const {
            return _file_path;
        }

        UniqueFile* operator->() {
            return &_files.at(_file_path);
        }

    private:
        fs::path _file_path{};
        inline static std::unordered_map<fs::path, UniqueFile> _files{};
    };

}