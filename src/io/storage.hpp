#pragma once 

#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

namespace cosmo::io {
    class Storage {
        struct StorageOptions{
            uint32_t _max_data_file_size {10 * 1024 * 1024};
            bool _auto_merge {true};
            uint8_t _max_non_active_data_file_number {5};

            StorageOptions() = default;

            StorageOptions(uint32_t max_data_file_size, bool auto_merge, uint8_t max_non_active_data_file_number) noexcept:
                _max_data_file_size(max_data_file_size), _auto_merge(auto_merge), _max_non_active_data_file_number(max_non_active_data_file_number)  
            {}
        };

        public:
            explicit Storage(const fs::path& directory_path): 
                _storage_directory{directory_path}{
                if(!_storage_directory.exists() || !_storage_directory.is_directory()){
                    throw std::invalid_argument("the path provided is not valid");
                }

                _data_files.reserve(_props._max_non_active_data_file_number);
            }

            std::optional<fs::path> getActiveFilePath();

        private:
            using file = std::fstream;

            static const auto APPEND_READ_WRITE = std::ios::app | std::ios::in | std::ios::out;

            StorageOptions _props;
            fs::directory_entry _storage_directory{};
            std::vector<file> _data_files{};
            file _active_data_file{};
            uint8_t _non_active_data_file_number{};
    };
}