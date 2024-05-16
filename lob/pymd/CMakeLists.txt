#pybind11_add_module(pymd pymd.cpp)

#add_library(pymd)

#target_sources(pymd PRIVATE pymd.cpp)

#pybind11_add_module(pymd pymd.cpp)
#target_link_libraries(pymd pybind11_all_do_not_use)


# Create an extension module
add_library(pymd MODULE pymd.cpp)
target_link_libraries(pymd PUBLIC pybind11::module)

# Or embed the Python interpreter into an executable
#add_executable(myexe main.cpp)
#target_link_libraries(myexe PUBLIC pybind11::embed)