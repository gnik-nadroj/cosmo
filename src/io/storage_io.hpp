#pragma once 


namespace cosmo::io {
	class Storage;

	class IStorageIo {
		public:
			virtual ReadResult read(Storage& storage, data_file_id file_id, offset pos, data_file_size size) = 0;
			virtual WriteResult write(Storage& storage, const std::string& value) = 0;

			virtual ~IStorageIo() = default;
	};
}