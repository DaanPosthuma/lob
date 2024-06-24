pybind11_add_module(pymd pymd.cpp)

target_link_libraries(pymd PRIVATE simulator)
