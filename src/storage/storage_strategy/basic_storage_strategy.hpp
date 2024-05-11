#pragma once

#include "storage_strategy.hpp"
#include "storage.hpp"

#include <shared_mutex>

namespace cosmo::storage {
    class BasicStorageStrategy : public IStorageStrategy {
        public:
            ReadResult read(Storage& storage, data_file_id file_id, offset pos, data_file_size size) override {
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

                storage.switchActiveDataFile();

                lck.unlock();

                auto [status, pos] = storage._active_data_file_stream.write(value.c_str(), value.size());
                storage._active_file_size += static_cast<data_file_size>(value.size());

                return { true, storage._active_file_id, pos }; 
            }

        private:
            std::shared_mutex _mtx;
    };
}