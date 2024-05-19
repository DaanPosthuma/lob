pybind11_add_module(pymd pymd.cpp ItchReader.cpp Condition.cpp)
pybind11_add_module(pymdf Condition.cpp)

target_link_libraries(pymd PRIVATE md)
