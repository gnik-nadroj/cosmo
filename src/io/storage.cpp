#include "storage.hpp"
#include "io_utils.hpp"
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <fmt/format.h>
#include <stdexcept>

namespace cosmo::io{

    bool Storage::read(data_file_id file_id, Offset pos, data_file_size size, char* buffer) {
        try {
            if (file_id == _active_file_id) {
                _active_data_file_stream->seekg(pos);
                _active_data_file_stream->read(buffer, size);
            }
            else {
                const auto& data_file = _data_files.at(file_id);
                FileHandle file{ data_file };

                file->seekg(pos);
                file->read(buffer, size);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Read operation failed: " << e.what() << '\n';
            return false;
        }

        return true;
    }

    std::tuple<bool, Storage::data_file_id, Storage::Offset> Storage::write(const std::string& value) {
        Offset pos;

        try {
            switchActiveDataFile();
            pos = _active_data_file_stream->tellp();
            _active_data_file_stream->write(value.c_str(), value.size());
            _active_file_size += static_cast<data_file_size>(value.size());
        }
        catch (const std::exception& e) {
            std::cerr << "Write operation failed: " << e.what() << '\n';
            return { false, _active_file_id, pos};
        }

        return { true, _active_file_id, pos };
    }

    void Storage::switchActiveDataFile() {
        if (_active_file_size >= _max_data_file_size) {
            _active_data_file_stream->close();
            auto new_data_file_name = _storage_directory.path() / getDataFileName(_active_file_id);
            fs::rename(_active_data_file_stream.getPath(), new_data_file_name);
            FileHandle new_active_data_file_stream{ _storage_directory.path() / getActiveFileName() };
            _active_data_file_stream = std::move(new_active_data_file_stream);
            _data_files.push_back(new_data_file_name);
            _active_file_id++;
        }
    }

    std::string Storage::getActiveFileName() {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        return fmt::format("{}_{}_{}", ACTIVE_FILE_PREFIX, std::to_string(now_c), FILE_EXTENSION);
    }

    std::string Storage::getDataFileName(data_file_id id) {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        return fmt::format("{}_{}_{}_{}", DATAFILE_PREFIX, id, std::to_string(now_c), FILE_EXTENSION);
    }
};