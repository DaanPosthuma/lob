#include "ItchReader.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <variant>

namespace py = pybind11;

PYBIND11_MODULE(pymd, m) {
  m.doc() = "market data plugin";

  py::class_<pymd::ItchReader>(m, "ItchReader")
      .def(py::init<std::string const&>())
      .def("onMessage", &pymd::ItchReader::onMessage)
      .def("read", [](pymd::ItchReader const& reader, int num){
        //py::gil_scoped_release release;
        return reader.read(num);
      });

}
