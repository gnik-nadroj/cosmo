#include "storage.hpp"
#include "io_utils.hpp"
#include <fstream>
#include <limits>
#include <memory>

namespace cosmo::io{
    void Storage::read(int file_id, int pos, size_t size, char* buffer) {
        if(file_id == ACTIVE_FILE_ID) {
            _active_data_file_stream->seekg(pos);
            _active_data_file_stream->read(buffer, size);
        } else {
            const auto& data_file = _data_files.at(file_id);
            FileHandle file { data_file};

            file->seekg(pos);
            file->read(buffer, size);
        }
    }
};