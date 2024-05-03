#include "io_utils.hpp"
#include "storage.hpp"
#include "test_utils.hpp"


#include <cstdint>
#include <fstream>
#include <gtest/gtest.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using cosmo::io::Storage;
using cosmo::io::FileHandle;
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

void testRead(Storage& storage, int fileIndex, int offset, int length, const std::string& expected) {
    auto buffer = std::make_unique<char[]>(length + 1);
    auto* rawBuffer = buffer.get();
    rawBuffer[length] = '\0';
    bool status = storage.read(fileIndex, offset, length, rawBuffer);
    EXPECT_TRUE(status);
    EXPECT_EQ(expected, std::string(rawBuffer));
}

TEST_F(CosmoTest, activeFilePresent) 
{
    TemporaryFile tempFile(directory, "active.cosmo");

    Storage storage { directory };

    EXPECT_TRUE(storage.isActiveFileOpen());
}

TEST_F(CosmoTest, activeFileNotPresent) 
{
    Storage storage { directory };

    EXPECT_TRUE(storage.isActiveFileOpen());
}

TEST_F(CosmoTest, dataFilePresent) 
{
    TemporaryFile file1{directory, "datafile.1"};
    TemporaryFile file2{directory, "datafile.2"};
    TemporaryFile file3{directory, "datafile.3"};

    Storage storage { directory };

    EXPECT_FALSE(storage.getDataFiles().empty()); 

    EXPECT_EQ(storage.getDataFiles().size(), 3);
}

TEST_F(CosmoTest, dataFileNotPresent) 
{
    Storage storage { directory };

    EXPECT_TRUE(storage.getDataFiles().empty()); 
}

TEST_F(CosmoTest, simpleRead) 
{
    TemporaryFile file1{directory, "datafile.1"};
    Storage storage { directory };

    std::fstream fs1 {file1.filePath, FileHandle::APPEND_READ};
    fs1 << "jayzprodigy";
    fs1.close();

    testRead(storage, 0, 0, 4, "jayz");
    testRead(storage, 0, 4, 7, "prodigy");
}

TEST_F(CosmoTest, readUnicodeData) 
{
    TemporaryFile file1{directory, "datafile.1"};
    Storage storage { directory };

    std::fstream fs1 {file1.filePath, FileHandle::APPEND_READ};
    fs1 << "jayzprodigy😘👌50CENT";
    fs1.close();

    testRead(storage, 0, 0, 4, "jayz");
    testRead(storage, 0, 4, 11, "prodigy😘");
}

TEST_F(CosmoTest, multipleFileRead) 
{
    TemporaryFile file1{directory, "datafile.1"};
    TemporaryFile file2{directory, "datafile.2"};
    TemporaryFile file3{directory, "datafile.3"};

    std::fstream fs1 {file1.filePath, FileHandle::APPEND_READ};
    std::fstream fs2 {file2.filePath, FileHandle::APPEND_READ};
    std::fstream fs3 {file3.filePath, FileHandle::APPEND_READ};

    fs1 << "jayzprodigy";
    fs2 << "gokuvegeta";
    fs3 << "jasonbrodycitra";

    fs1.close();
    fs2.close();
    fs3.close();

    Storage storage { directory };

    int file1_id{};
    int file2_id{};
    int file3_id{};

    for(auto i = 0; i < storage.getDataFiles().size(); ++i) {
        auto& path = storage.getDataFiles().at(i);

        if(path == file1.filePath) {
            file1_id = i; 
        } else if(path == file2.filePath) {
             file2_id = i;
        } else {
            file3_id = i;
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

    EXPECT_EQ(storage.getActiveFileInputPosition(), value.size());
}

TEST_F(CosmoTest, writeEmptyString)
{
    Storage storage{ directory };

    std::string value = "";

    auto [writeSuccess, id, pos] = storage.write(value);

    EXPECT_TRUE(writeSuccess);

    EXPECT_EQ(id, 0);

    EXPECT_EQ(pos, 0);

    EXPECT_EQ(storage.getActiveFileInputPosition(), 0);
}

TEST_F(CosmoTest, writeLargeString)
{
    Storage storage{ directory };

    std::string value(1'000'000, 'a');

    auto [writeSuccess, id, pos] = storage.write(value);

    EXPECT_TRUE(writeSuccess);

    EXPECT_EQ(id, 0);

    EXPECT_EQ(pos, 0);

    EXPECT_EQ(storage.getActiveFileInputPosition(), value.size());
}

TEST_F(CosmoTest, writeSpecialCharacters)
{
    Storage storage{ directory };

    std::string value = "Hello, world! 😊👍🌍";

    auto [writeSuccess, id, pos] = storage.write(value);

    EXPECT_TRUE(writeSuccess);

    EXPECT_EQ(id, 0);

    EXPECT_EQ(pos, 0);

    EXPECT_EQ(storage.getActiveFileInputPosition(), value.size());
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

    for (int i = 0; i < values.size(); ++i) {
        auto [writeSuccess, id, pos] = storage.write(values[i]);

        EXPECT_TRUE(writeSuccess);
        EXPECT_EQ(pos, expectedPos);

        expectedPos += values[i].size();
        EXPECT_EQ(storage.getActiveFileInputPosition(), expectedPos);
    }
}

TEST_F(CosmoTest, writeMultipleValuesAndSwitchFile)
{
    Storage::data_file_size maxFileSize = 20;
    Storage storage{ directory, maxFileSize };

    std::vector<std::string> values = {
        "Hello, world! 😊👍🌍",
        "CosmoTest is great! 🚀🌌",
        "Multiple writes test! 📝🔄"
    };

    std::streampos expectedPos = 0;
    Storage::data_file_id expectedFileId = 0;

    for (int i = 0; i < values.size(); ++i) {
        auto [writeSuccess, id, pos] = storage.write(values[i]);

        EXPECT_TRUE(writeSuccess);
        EXPECT_EQ(pos, expectedPos);
        EXPECT_EQ(id, expectedFileId);

        expectedPos += values[i].size();

        EXPECT_EQ(storage.getActiveFileInputPosition(), expectedPos);


        if (expectedPos >= maxFileSize) {
            expectedPos = 0;
            expectedFileId++;
        } 
    }
}



