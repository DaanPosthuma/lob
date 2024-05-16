add_library(md)

find_package(Boost REQUIRED)

target_sources(md PUBLIC itch_reader.h PRIVATE itch_reader.cpp)

target_link_libraries(md ${Boost_LIBRARIES})


target_include_directories(md PRIVATE ${Boost_INCLUDE_DIRS} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
