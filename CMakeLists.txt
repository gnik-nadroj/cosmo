cmake_minimum_required(VERSION 3.20.0)

project(cosmo VERSION 1.0.0 LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(NoInSourceBuilds)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(io STATIC src/io/io_utils.cpp src/io/storage.cpp src/io/storage_io.hpp src/io/basic_storage_io.hpp)
target_include_directories(io PUBLIC src/io)
target_link_libraries(io PRIVATE fmt::fmt-header-only)

add_library(cosmo src/cosmo.cpp)
target_link_libraries(cosmo PRIVATE io)
target_include_directories(cosmo PUBLIC include)

add_subdirectory(tests)
add_subdirectory(dependencies/fmt)