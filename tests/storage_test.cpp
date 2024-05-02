#include "storage.hpp"
#include "test_utils.hpp"


#include <cstdint>
#include <fstream>
#include <gtest/gtest.h>
#include <filesystem>
#include <iostream>

using cosmo::io::Storage;

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
    storage.read(fileIndex, offset, length, rawBuffer);
    EXPECT_EQ(expected, std::string(rawBuffer));
}

TEST_F(CosmoTest, activeFilePresent) 
{
    TemporaryFile tempFile(directory, "active.cosmo");

    Storage storage { directory };

    EXPECT_TRUE(storage.isActiveFileOpen());
    EXPECT_EQ(storage.getActiveFilePath(), directory / "active.cosmo"); 
}

TEST_F(CosmoTest, activeFileNotPresent) 
{
    Storage storage { directory };

    EXPECT_TRUE(storage.isActiveFileOpen());
    EXPECT_EQ(storage.getActiveFilePath(), directory / "active.cosmo"); 
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

    std::fstream fs1 {file1.filePath, Storage::APPEND_READ};
    fs1 << "jayzprodigy";
    fs1.close();

    testRead(storage, 0, 0, 4, "jayz");
    testRead(storage, 0, 4, 7, "prodigy");
}

TEST_F(CosmoTest, multipleFileRead) 
{
    TemporaryFile file1{directory, "datafile.1"};
    TemporaryFile file2{directory, "datafile.2"};
    TemporaryFile file3{directory, "datafile.3"};

    std::fstream fs1 {file1.filePath, Storage::APPEND_READ};
    std::fstream fs2 {file2.filePath, Storage::APPEND_READ};
    std::fstream fs3 {file3.filePath, Storage::APPEND_READ};

    fs1 << "jayzprodigy";
    fs2 << "gokuvegeta";
    fs3 << "jasonbrodycitra";

    fs1.close();
    fs2.close();
    fs3.close();

    Storage storage { directory };

    int file1_id;
    int file2_id;
    int file3_id;

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
