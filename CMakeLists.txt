cmake_minimum_required(VERSION 3.20.0)

project(cosmo VERSION 1.0.0 LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(NoInSourceBuilds)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(io STATIC src/io/storage.cpp)
target_include_directories(io PUBLIC src/io)

add_library(cosmo src/cosmo.cpp)
target_link_libraries(cosmo PUBLIC io)
target_include_directories(cosmo PUBLIC include)

add_subdirectory(tests)