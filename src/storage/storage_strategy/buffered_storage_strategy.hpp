#pragma once

#include "storage_strategy.hpp"

#include <storage.hpp>

#include <shared_mutex>
#include <string> 

namespace cosmo::storage {
    class BufferedStorageStrategy : public IStorageStrategy {
    public:
        explicit BufferedStorageStrategy(size_t max_buffer_size) {
            _buffer.reserve(max_buffer_size);
        }

        //will be improved to read from the _buffer id data is present in it.
        ReadResult read(Storage& storage, data_file_id_t file_id, offset_t pos, data_file_size_t size) override {
            std::shared_lock lck{ _mtx };

            if (file_id == storage._active_file_id) {
                return storage._active_data_file_stream.read(pos, size);
            }
            else {
                return storage._data_files.at(file_id).read(pos, size);
            }
        }

        WriteResult write(Storage& storage, const std::string& value) override {
            std::unique_lock lck {_mtx};

            if (_buffer.size() + value.size() > _buffer.capacity()) {
                storage._active_file_id++;
                storage._active_data_file_stream.write(_buffer.data(), _buffer.size());
                _buffer.clear();
                storage.switchActiveDataFile();
            }

            auto pos = _buffer.size();
            _buffer.append(value);

            lck.unlock();

            return { true, storage._active_file_id, pos };
        }

    private:
        std::shared_mutex _mtx;
        std::string _buffer{};
    };


}

