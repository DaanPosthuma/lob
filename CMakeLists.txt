cmake_minimum_required(VERSION 3.22)

project(lob
    VERSION 0.0.1
    DESCRIPTION "todo"
    LANGUAGES CXX
)

message("Building with CMake version: ${CMAKE_VERSION}")

enable_testing()

find_package(GTest REQUIRED)
find_package(PyBind11 REQUIRED)

include(GoogleTest)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#if(MSVC)
#  add_compile_options(/W4 /WX)
#else()
#  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
#endif()

add_subdirectory(lob)
