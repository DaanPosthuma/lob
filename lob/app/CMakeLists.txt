add_executable(app main.cpp)
target_link_libraries(app PRIVATE lob md STDEXEC::stdexec)
