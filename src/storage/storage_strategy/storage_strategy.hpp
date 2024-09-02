#pragma once 

#include <storage_utils.hpp>


namespace cosmo::storage {
	class Storage;

	class IStorageStrategy {
		public:
			virtual ReadResult read(Storage& storage, data_file_id_t file_id, offset_t pos, data_file_size_t size) = 0;
			virtual WriteResult write(Storage& storage, const std::string& value) = 0;

			virtual ~IStorageStrategy() = default;
	};
}