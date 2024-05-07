#pragma once 

#include "io_utils.hpp"
#include "storage_io.hpp"

#include <ranges>
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
        explicit Storage(const fs::path& directory_path, data_file_size max_data_file_size = DEFAULT_MAX_DATA_FILE_SIZE);

        ReadResult read(data_file_id file_id, offset pos, data_file_size size);

        WriteResult write(const std::string& value);
            
        const std::vector<fs::path>& getDataFiles() const { return _data_files; };
            
        bool isActiveFileOpen() const { return _active_data_file_stream.isOpen(); };

        const fs::directory_entry& getStorageDirectory() const { return _storage_directory; }

        data_file_id getActiveFileId() const { return _active_file_id; }

        data_file_size getActiveFileSize() const { return _active_file_size.load(); }

        data_file_size getMaxDataFileSize() const { return _max_data_file_size; }

    private:
        std::string getActiveFileName() const;
        std::string getDataFileName(data_file_id id) const;
        void switchActiveDataFile();

        fs::directory_entry _storage_directory{};
        std::vector<fs::path> _data_files{};
        fs::path _active_file_path{};
        UniqueFile _active_data_file_stream{};
        data_file_id _active_file_id{};
        std::atomic_uint32_t _active_file_size{};
        data_file_size _max_data_file_size{};

        std::unique_ptr<IStorageIo> _store;

        inline static const std::string ACTIVE_FILE_PREFIX{ "activefile" };
        inline static const std::string DATAFILE_PREFIX{ "datafile" };
        inline static const std::string FILE_EXTENSION{ ".cosmo" };

        static const data_file_size DEFAULT_MAX_DATA_FILE_SIZE{ 10 * 1024 * 1024 };
        static const data_file_size DEFAULT_MAX_DATA_FILE_NUMBER{ 255 };

        friend class BasicStorageIo;
    };
}