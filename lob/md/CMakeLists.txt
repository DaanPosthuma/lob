add_library(md)

target_sources(md PUBLIC MappedFile.h PRIVATE MappedFile.cpp)

target_link_libraries(md ${Boost_LIBRARIES})

target_include_directories(md 
	PRIVATE 
		${Boost_INCLUDE_DIRS}
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}/..
)

