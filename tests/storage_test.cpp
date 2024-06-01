#include <storage_utils.hpp>
#include "storage.hpp"
#include "test_utils.hpp"


#include <cstdint>
#include <numeric>
#include <fstream>
#include <gtest/gtest.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <cstdio>

using cosmo::storage::Storage;
class CosmoTest : public testing::Test {
public:
    std::filesystem::path directory{};

    void SetUp() override {
        directory = std::filesystem::temp_directory_path() / "cosmo_test";

        std::filesystem::create_directories(directory);
    }

    void TearDown() override {
        std::filesystem::remove_all(directory);
    }
};


auto testRead(Storage& storage, cosmo::storage::data_file_id fileIndex, cosmo::storage::offset offset, cosmo::storage::data_file_size length, const std::string& expected) {
    auto start = std::chrono::high_resolution_clock::now();
    auto [status, buffer] = storage.read(fileIndex, offset, length);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    EXPECT_TRUE(status);
    auto str = std::string(buffer);
    EXPECT_EQ(expected, str);

    return elapsed.count();
}

TEST_F(CosmoTest, activeFilePresent)
{
    TemporaryFile tempFile(directory, "active.cosmo");

    Storage storage{ directory };

    EXPECT_TRUE(storage.isActiveFileOpen());
}

TEST_F(CosmoTest, activeFileNotPresent)
{
    Storage storage{ directory };

    EXPECT_TRUE(storage.isActiveFileOpen());
}

TEST_F(CosmoTest, dataFilePresent)
{
    TemporaryFile file1{ directory, "datafile.1" };
    TemporaryFile file2{ directory, "datafile.2" };
    TemporaryFile file3{ directory, "datafile.3" };

    Storage storage{ directory };

    EXPECT_FALSE(storage.getDataFiles().empty());

    EXPECT_EQ(storage.getDataFiles().size(), 3);
}

TEST_F(CosmoTest, dataFileNotPresent)
{
    Storage storage{ directory };

    EXPECT_TRUE(storage.getDataFiles().empty());
}

TEST_F(CosmoTest, simpleRead)
{
    TemporaryFile file1{ directory, "datafile.1" };
    Storage storage{ directory };

    std::fstream fs1{ file1.filePath, cosmo::storage::APPEND_READ };
    fs1 << "jayzprodigy";
    fs1.close();

    testRead(storage, 0, 0, 4, "jayz");
    testRead(storage, 0, 4, 7, "prodigy");
}

TEST_F(CosmoTest, readLargeData)
{
    TemporaryFile file1{ directory, "datafile.1" };
    Storage storage{ directory };

    std::fstream fs1{ file1.filePath, cosmo::storage::APPEND_READ };
    std::string value(100'000, 'a');
    fs1 << value;
    fs1.close();

    testRead(storage, 0, 0, 100'000, value);
    testRead(storage, 0, 0, 100'000, value);
}

TEST_F(CosmoTest, readUnicodeData)
{
    TemporaryFile file1{ directory, "datafile.1" };
    Storage storage{ directory };

    std::fstream fs1{ file1.filePath, cosmo::storage::APPEND_READ };
    fs1 << "jayzprodigy😘👌50CENT";
    fs1.close();

    testRead(storage, 0, 0, 4, "jayz");
    testRead(storage, 0, 4, 11, "prodigy😘");
}

TEST_F(CosmoTest, multipleFileRead)
{
    TemporaryFile file1{ directory, "datafile.1" };
    TemporaryFile file2{ directory, "datafile.2" };
    TemporaryFile file3{ directory, "datafile.3" };

    std::fstream fs1{ file1.filePath, cosmo::storage::APPEND_READ };
    std::fstream fs2{ file2.filePath, cosmo::storage::APPEND_READ };
    std::fstream fs3{ file3.filePath, cosmo::storage::APPEND_READ };

    fs1 << "jayzprodigy";
    fs2 << "gokuvegeta";
    fs3 << "jasonbrodycitra";

    fs1.close();
    fs2.close();
    fs3.close();

    Storage storage{ directory };

    cosmo::storage::data_file_id file1_id{};
    cosmo::storage::data_file_id file2_id{};
    cosmo::storage::data_file_id file3_id{};

    for (auto i = 0; i < storage.getDataFiles().size(); ++i) {
        auto& path = storage.getDataFiles().at(i);

        auto id = static_cast<cosmo::storage::data_file_id>(i);

        if (path.getPath() == file1.filePath) {
            file1_id = id;
        }
        else if (path.getPath() == file2.filePath) {
            file2_id = id;
        }
        else {
            file3_id = id;
        }
    }

    testRead(storage, file1_id, 0, 4, "jayz");
    testRead(storage, file3_id, 10, 5, "citra");
    testRead(storage, file2_id, 4, 6, "vegeta");
}

TEST_F(CosmoTest, simpleWrite)
{
    Storage storage{ directory };

    std::string value = "Hello, world!";

    auto [writeSuccess, id, pos] = storage.write(value);

    EXPECT_TRUE(writeSuccess);

    EXPECT_EQ(id, 0);

    EXPECT_EQ(pos, 0);
}

TEST_F(CosmoTest, writeEmptyString)
{
    Storage storage{ directory };

    std::string value = "";

    auto [writeSuccess, id, pos] = storage.write(value);

    EXPECT_TRUE(writeSuccess);

    EXPECT_EQ(id, 0);

    EXPECT_EQ(pos, 0);
}

TEST_F(CosmoTest, writeLargeString)
{
    Storage storage{ directory };

    std::string value(1'000'000, 'a');

    auto [writeSuccess, id, pos] = storage.write(value);

    EXPECT_TRUE(writeSuccess);

    EXPECT_EQ(id, 0);

    EXPECT_EQ(pos, 0);
}

TEST_F(CosmoTest, writeSpecialCharacters)
{
    Storage storage{ directory };

    std::string value = "Hello, world! 😊👍🌍";

    auto [writeSuccess, id, pos] = storage.write(value);

    EXPECT_TRUE(writeSuccess);

    EXPECT_EQ(id, 0);

    EXPECT_EQ(pos, 0);
}

TEST_F(CosmoTest, writeMultipleValues)
{
    Storage storage{ directory };

    std::vector<std::string> values = {
        "Hello, world! 😊👍🌍",
        "CosmoTest is great! 🚀🌌",
        "Multiple writes test! 📝🔄"
    };

    std::streampos expectedPos = 0;

    for (const auto& value : values) {
        auto [writeSuccess, id, pos] = storage.write(value);

        EXPECT_TRUE(writeSuccess);
        EXPECT_EQ(pos, expectedPos);

        expectedPos += value.size();
    }
}
