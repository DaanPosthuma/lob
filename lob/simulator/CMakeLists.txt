add_library(simulator functions.cpp)
target_link_libraries(simulator PUBLIC md PRIVATE strategies lob)
