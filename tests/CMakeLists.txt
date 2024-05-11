include(Testing)

add_executable(tests storage_test.cpp)

target_link_libraries(tests PUBLIC storage)

AddTests(tests)