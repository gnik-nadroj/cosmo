#pragma once 

#include "io_utils.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
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

                FileHandle f {directory_path / ACTIVE_FILE_NAME};
                _active_data_file_stream = std::move(f);
                _data_files.reserve(100);
                _data_files = seachFiles(_storage_directory, DATAFILE_PREFIX);
            }

            void read(int file_id, int pos, size_t size, char* buffer);

            void write();
            
            const std::vector<fs::path>& getDataFiles() const { return _data_files; };
            
            bool isActiveFileOpen() { return _active_data_file_stream->is_open(); };
        private:
            StorageOptions _props;
            fs::directory_entry _storage_directory;
            std::vector<fs::path> _data_files;
            FileHandle _active_data_file_stream;
            uint8_t _non_active_data_file_number;

            static const uint8_t ACTIVE_FILE_ID = 255;
            inline static const std::string ACTIVE_FILE_NAME = "activefile";
            inline static const std::string DATAFILE_PREFIX = "datafile";
    };
}