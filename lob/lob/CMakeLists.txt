#message("OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
#message("Boost include dir: ${BOOST_INCLUDE_DIR}")
#message("PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

add_library(lob)

target_sources(lob PUBLIC lob.h PRIVATE lob.cpp bufferedreader.h)

set_target_properties(lob PROPERTIES FOLDER lob)

target_include_directories(lob PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

#target_compile_options(lob
#    INTERFACE
#        $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
#)
