cmake_minimum_required(VERSION 3.20.0)

project(cosmo VERSION 1.0.0 LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(NoInSourceBuilds)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(storage STATIC "src/storage/utils/storage_utils.cpp" "src/storage/storage.cpp" "src/storage/storage_strategy/storage_strategy.hpp" "src/storage/storage_strategy/basic_storage_strategy.hpp" "src/storage/storage_strategy/buffered_storage_strategy.hpp")
target_include_directories(storage PUBLIC "src/storage" PUBLIC "src/storage/utils")
target_link_libraries(storage PRIVATE fmt::fmt-header-only)

add_library(cosmo src/cosmo.cpp)
target_link_libraries(cosmo PRIVATE storage)
target_include_directories(cosmo PUBLIC include)

add_subdirectory(tests)
add_subdirectory(dependencies/fmt)