add_executable(LOBTests LOBTests.cpp)
target_link_libraries(LOBTests PRIVATE GTest::GTest GTest::Main lob)
gtest_discover_tests(LOBTests)
