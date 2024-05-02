#include "storage.hpp"
#include "io_utils.hpp"
#include <fstream>
#include <limits>
#include <memory>

namespace cosmo::io{
    void Storage::setupActiveFile() {
        auto active_file_path = searchFile(_storage_directory, "active.cosmo");

        if(active_file_path.has_value()) {
            std::unique_ptr<File, FileStreamDeleter> f {new File(active_file_path->string(), APPEND_READ)};
            _active_data_file_stream = std::move(f);
            _active_file_path = active_file_path.value();
        } else {
            auto file_path = _storage_directory.path() / "active.cosmo";
            std::unique_ptr<File, FileStreamDeleter> f {new File(file_path.string(), APPEND_READ)};
            _active_data_file_stream = std::move(f);
            _active_file_path = file_path;
        }
    }

    void Storage::setupDataFiles() {
        _data_files = seachFiles(_storage_directory, "datafile");
    };

    void Storage::open() {
        setupActiveFile();
        setupDataFiles();
    }

    void Storage::read(int file_id, int pos, size_t size, char* buffer) {
        if(file_id == ACTIVE_FILE_ID) {
            _active_data_file_stream->seekg(pos);
            _active_data_file_stream->read(buffer, size);
        } else {
            const auto& data_file = _data_files.at(file_id);
            File file { data_file, APPEND_READ };

            if(!file.is_open()) {
                for(int i = 0;i < size; i++) {
                    buffer[i] = 'Z';
                }
                return;
            }

            file.seekg(pos);
            file.read(buffer, size);

            file.close();
        }
    }
};