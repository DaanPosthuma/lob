cmake_minimum_required(VERSION 3.22)

project(lob
    VERSION 0.0.1
    DESCRIPTION "todo"
    LANGUAGES CXX
)

message("Building with CMake version: ${CMAKE_VERSION}")

# download CPM.cmake
file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

enable_testing()

find_package(GTest REQUIRED)
find_package(Boost REQUIRED iostreams)
find_package(Python COMPONENTS Interpreter Development)
find_package(pybind11 CONFIG)

CPMAddPackage(
  NAME stdexec
  GITHUB_REPOSITORY NVIDIA/stdexec
  GIT_TAG main
)

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
