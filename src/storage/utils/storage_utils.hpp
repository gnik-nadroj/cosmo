#pragma once


#include <optional>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <iostream>
#include <shared_mutex>
#include <functional>
#include <atomic>

namespace fs = std::filesystem;

namespace cosmo::storage {
	using offset_t = std::streampos;
	using data_file_size_t = uint32_t;
	using data_file_id_t = uint32_t;

	using ReadResult = std::pair<bool, char*>;
	using WriteResult = std::tuple<bool, data_file_id_t, offset_t>;

	inline static const auto APPEND_READ = std::ios::app | std::ios::in;

	std::optional<fs::path> searchFile(const fs::directory_entry& directory, const std::string_view filename);
	std::vector<fs::path> seachFiles(const fs::directory_entry& directory, const std::string_view filename);

	template <typename Func, typename ReturnType = std::invoke_result_t<Func>>
	std::pair<bool, ReturnType> safeIoOperation(Func func) {
		try {
			return { true, func() };
		}
		catch (const std::filesystem::filesystem_error& e) {
			std::cerr << "Filesystem error: " << e.what() << '\n';
		}
		catch (const std::ios_base::failure& e) {
			std::cerr << "I/O error: " << e.what() << '\n';
		}
		catch (const std::bad_alloc& e) {
			std::cerr << "Memory allocation error: " << e.what() << '\n';
		}
		catch (const std::exception& e) {
			std::cerr << "General error: " << e.what() << '\n';
		}
		return { false, ReturnType{} };
	}

	class CharBuffer {
	private:
		std::mutex _mtx;
		uint32_t _size{ 1'000'000'000 };
		std::unique_ptr<char[]> _buffer{};
		uint32_t _current_pos{};

	public:
		CharBuffer() : _buffer{ std::make_unique<char[]>(_size) } {};
		explicit CharBuffer(uint32_t size) : _size{ size }, _buffer{ std::make_unique<char[]>(size) } {};
		char* getBuffer(std::streamsize requestedSize) {
			std::scoped_lock lck{ _mtx };

			if (requestedSize > _size) {
			   return nullptr;
			}

			if (_current_pos + requestedSize >= _size) {
				_current_pos = 0;
			}

			char* start = &_buffer[_current_pos];
			_buffer[_current_pos + requestedSize] = '\0';
			_current_pos += requestedSize + 1;

			return start;
		}
	};

	class ConcurrentFile {
	public:
		ConcurrentFile() = default;

		~ConcurrentFile() = default;

		ConcurrentFile(const ConcurrentFile&) = delete;
		ConcurrentFile& operator=(const ConcurrentFile&) = delete;

		ConcurrentFile(const fs::path& filePath, std::ios_base::openmode mode = APPEND_READ)
			: _file_path{ filePath }, _writer{ filePath, mode } { 
			if (!_writer.is_open()) {
				throw std::invalid_argument("Unable to in file");
			}

			_current_write_pos = _writer.tellp();
		}

		ConcurrentFile(ConcurrentFile&& other) noexcept {
			std::scoped_lock lock{ _mtx, other._mtx };
			_file_path = std::move(other._file_path);
			_writer = std::move(other._writer);
		}

		ConcurrentFile& operator=(ConcurrentFile&& other) noexcept {
			if (this != &other) {
				std::scoped_lock lock{ _mtx, other._mtx };
				_file_path = std::move(other._file_path);
				_writer = std::move(other._writer);
			}
			return *this;
		}

		
		std::pair<bool, char*> read(offset_t offset, std::streamsize size) const {
			return safeIoOperation([this, &size, &offset] {
				auto buffer = _char_buffer.getBuffer(size);

				if(offset + size < _current_write_pos) {
					readInternal(buffer, offset, size);
				} else {
					std::shared_lock lck{ _mtx };
					readInternal(buffer, offset, size);
				}
				
				return buffer;
			});
		}

		std::pair<bool, offset_t> write(const char* value, std::streamsize size) {
			return safeIoOperation([this, &value, &size] {
				std::scoped_lock lck{ _mtx };

				auto pos = _current_write_pos;
				
				_writer.write(value, size);

				_current_write_pos = _writer.tellp();

				return pos;
			});
		}

		bool isOpen() const {
			return _writer.is_open();
		}

		const fs::path& getPath() const {
			return _file_path;
		}

	private:
		fs::path _file_path{};
		offset_t _current_write_pos{};
		std::fstream _writer{};
		mutable std::shared_mutex _mtx;

		inline static CharBuffer _char_buffer{};

		void readInternal(char* buffer, offset_t offset, std::streamsize size) const {
			std::ifstream reader{ _file_path, std::ios::in };

			reader.seekg(offset);

			reader.read(buffer, size);
		}
	};
}