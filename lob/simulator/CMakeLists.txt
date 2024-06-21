add_library(simulator functions.cpp Simulator.cpp)
target_link_libraries(simulator PUBLIC md PRIVATE strategies lob)
