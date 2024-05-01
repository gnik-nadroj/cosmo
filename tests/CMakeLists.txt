include(Testing)

add_executable(tests storage_test.cpp)

target_link_libraries(tests PUBLIC io)

AddTests(tests)