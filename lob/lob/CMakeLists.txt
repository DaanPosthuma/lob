add_library(lob)

target_sources(lob PUBLIC lob.h PRIVATE lob.cpp)

target_include_directories(lob PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
