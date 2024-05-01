#include "storage.hpp"
#include "test_utils.hpp"


#include <gtest/gtest.h>
#include <filesystem>


using cosmo::io::Storage;

class CosmoTest : public testing::Test {
protected:
    std::filesystem::path directory{};

    void SetUp() override {
        directory = std::filesystem::temp_directory_path() / "cosmo_test";

        std::filesystem::create_directories(directory);
    }

    void TearDown() override {
        std::filesystem::remove_all(directory);
    }
};

TEST_F(CosmoTest, activeFilePresent) 
{
    TemporaryFile tempFile(directory, "active");

    Storage storage { directory };

    auto result = storage.getActiveFilePath();

    EXPECT_TRUE(result.has_value()); 
}

TEST_F(CosmoTest, activeFileNotPresent) 
{
    Storage storage { directory };

    auto result = storage.getActiveFilePath();

    EXPECT_FALSE(result.has_value()); 
}