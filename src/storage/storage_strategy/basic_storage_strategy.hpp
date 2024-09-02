#pragma once

#include "storage_strategy.hpp"
#include "storage.hpp"

#include <shared_mutex>

namespace cosmo::storage {
    class BasicStorageStrategy : public IStorageStrategy {
        public:
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
                std::unique_lock lck{ _mtx };

                if (storage._active_file_size.load() >= storage._max_data_file_size) {
                    storage._active_file_id++;
                    storage.switchActiveDataFile();
                    storage._active_file_size = 0;
                }

                lck.unlock();

                auto [status, pos] = storage._active_data_file_stream.write(value.c_str(), value.size());
                auto value_size = status ? static_cast<data_file_size_t>(value.size()) : 0;
                storage._active_file_size += value_size;

                return { status, storage._active_file_id, pos }; 
            }

        private:
            std::shared_mutex _mtx;
    };
}