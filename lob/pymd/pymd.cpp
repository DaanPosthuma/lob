#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <functional>
#include <string>
#include <unordered_map>

namespace py = pybind11;

PYBIND11_MODULE(pymd, m) {
  m.doc() = "trading simulator";
}
