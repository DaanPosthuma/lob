add_library(simulator functions.cpp Simulator.cpp ItchBooksManager.cpp)
target_link_libraries(simulator PUBLIC md PRIVATE strategies lob STDEXEC::stdexec)
