#pragma once 

#include "io_utils.hpp"

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
        public:
            using offset = std::fstream::pos_type;
            using data_file_size = uint32_t;
            using data_file_id = uint8_t;

            explicit Storage(const fs::path& directory_path, data_file_size max_data_file_size = DEFAULT_MAX_DATA_FILE_SIZE):
                _storage_directory{ directory_path }, _max_data_file_size{ max_data_file_size } {
                if(!_storage_directory.exists() || !_storage_directory.is_directory()){
                    throw std::invalid_argument("the path provided is not valid");
                }

                auto active_file_path = directory_path / getActiveFileName();
                _active_data_file_stream = FileHandle{ active_file_path };
                _active_file_size = static_cast<data_file_size>(fs::file_size(active_file_path));
                _data_files = seachFiles(_storage_directory, DATAFILE_PREFIX);
                _data_files.reserve(DEFAULT_MAX_DATA_FILE_NUMBER);
                _active_file_id = static_cast<data_file_id>(_data_files.size());
            }

            bool read(data_file_id file_id, offset pos, data_file_size size, char* buffer);

            std::tuple<bool, data_file_id, offset> write(const std::string& value);
            
            const std::vector<fs::path>& getDataFiles() const { return _data_files; };

            std::streamsize getActiveFileInputPosition() { return _active_data_file_stream->tellg(); };
            
            bool isActiveFileOpen() { return _active_data_file_stream->is_open(); };

        private:
            std::string getActiveFileName() const;
            std::string getDataFileName(data_file_id id) const;
            void switchActiveDataFile();

            fs::directory_entry _storage_directory{};
            std::vector<fs::path> _data_files{};
            FileHandle _active_data_file_stream{};
            data_file_id _active_file_id{};
            data_file_size _active_file_size{};
            data_file_size _max_data_file_size{};

            inline static const std::string ACTIVE_FILE_PREFIX{ "activefile" };
            inline static const std::string DATAFILE_PREFIX{ "datafile" };
            inline static const std::string FILE_EXTENSION{ ".cosmo" };

            static const data_file_size DEFAULT_MAX_DATA_FILE_SIZE{ 10 * 1024 * 1024 };
            static const data_file_size DEFAULT_MAX_DATA_FILE_NUMBER{ 255 };

    };
}