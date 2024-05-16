add_executable(LOBTests lob.tests.cpp md.tests.cpp)
target_link_libraries(LOBTests PRIVATE GTest::GTest GTest::Main lob md)
gtest_discover_tests(LOBTests)
