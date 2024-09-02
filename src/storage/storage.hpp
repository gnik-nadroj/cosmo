#pragma once 

#include "utils/storage_utils.hpp"
#include "storage_strategy/storage_strategy.hpp"

#include <atomic>
#include <ranges>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

namespace cosmo::storage {
    class Storage {
    public:
        explicit Storage(const fs::path& directory_path, data_file_size_t max_data_file_size = DEFAULT_MAX_DATA_FILE_SIZE);

        ReadResult read(data_file_id_t file_id, offset_t pos, data_file_size_t size);

        WriteResult write(const std::string& value);
            
        const std::vector<ConcurrentFile>& getDataFiles() const { return _data_files; };
            
        bool isActiveFileOpen() const { return _active_data_file_stream.isOpen(); };

        const fs::directory_entry& getStorageDirectory() const { return _storage_directory; }

        data_file_id_t getActiveFileId() const { return _active_file_id; }

        data_file_size_t getActiveFileSize() const { return _active_file_size.load(); }

        data_file_size_t getMaxDataFileSize() const { return _max_data_file_size; }

    private:
        std::string getActiveFileName() const;
        std::string getDataFileName(data_file_id_t id) const;
        void switchActiveDataFile();

        fs::directory_entry _storage_directory{};
        std::vector<ConcurrentFile> _data_files{};
        ConcurrentFile _active_data_file_stream{};
        data_file_id_t _active_file_id{};
        std::atomic<data_file_size_t> _active_file_size{};
        data_file_size_t _max_data_file_size{};

        std::unique_ptr<IStorageStrategy> _store;

        inline static const std::string ACTIVE_FILE_PREFIX{ "activefile" };
        inline static const std::string DATAFILE_PREFIX{ "datafile" };
        inline static const std::string FILE_EXTENSION{ ".cosmo" };

        static const data_file_size_t DEFAULT_MAX_DATA_FILE_SIZE{ 1'000'000'000 };
        static const data_file_size_t DEFAULT_MAX_DATA_FILE_NUMBER{ 4'000 };

        friend class BasicStorageStrategy;
        friend class BufferedStorageStrategy;
    };
}