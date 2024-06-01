#include "storage.hpp"


#include "storage_strategy/basic_storage_strategy.hpp"
#include "storage_strategy/buffered_storage_strategy.hpp"

#include <fstream>
#include <fmt/format.h>
#include <iostream>
#include <limits>
#include <memory>

namespace cosmo::storage{
    Storage::Storage(const fs::path& directory_path, data_file_size max_data_file_size):
        _storage_directory{ directory_path }, _max_data_file_size{ max_data_file_size } {

        if (!_storage_directory.exists() || !_storage_directory.is_directory()) {
            throw std::invalid_argument("the path provided is not valid");
        }

        _data_files.reserve(DEFAULT_MAX_DATA_FILE_NUMBER);
        auto existing_data_files = seachFiles(_storage_directory, DATAFILE_PREFIX);
        std::ranges::copy(existing_data_files, std::back_inserter(_data_files));
        _active_file_id = static_cast<data_file_id>(_data_files.size());

        _active_data_file_stream = ConcurrentFile{ directory_path / getActiveFileName() };
        _store = std::make_unique<BufferedStorageStrategy>(max_data_file_size);

        _active_file_size = static_cast<data_file_size>(fs::file_size(_active_data_file_stream.getPath()));
    }

    ReadResult Storage::read(data_file_id file_id, offset pos, data_file_size size) {
        return _store->read(*this, file_id, pos, size);
    }

    WriteResult Storage::write(const std::string& value) {
        return _store->write(*this, value);
    }

    void Storage::switchActiveDataFile() {
        auto old_active_file_path = _active_data_file_stream.getPath();
        ConcurrentFile new_active_data_file_stream{ _storage_directory.path() / getActiveFileName() };
        _active_data_file_stream = std::move(new_active_data_file_stream);
        auto new_data_file_name = _storage_directory.path() / getDataFileName(_active_file_id - 1);
        fs::rename(old_active_file_path, new_data_file_name);
        _data_files.emplace_back(new_data_file_name);
    }

    std::string Storage::getActiveFileName() const {
        return fmt::format("{}_{}{}", ACTIVE_FILE_PREFIX, _active_file_id, FILE_EXTENSION);
    }

    std::string Storage::getDataFileName(data_file_id id) const {
        return fmt::format("{}_{}{}", DATAFILE_PREFIX, id, FILE_EXTENSION);
    }
};