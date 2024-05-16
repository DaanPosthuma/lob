#include <pybind11/pybind11.h>
#include <md/itch_reader.h>
#include <string>

namespace py = pybind11;

namespace {

  void readFile(std::string& filename) {
    itch_reader::read(filename);
  }
}

PYBIND11_MODULE(pymd, m) {
  m.doc() = "market data plugin";
  m.def("read", &readFile);

  py::class_<itch_reader::FileReader>(m, "FileReader")
    .def(py::init<std::string const&>())
    .def("next", &itch_reader::FileReader::next)
    .def("currentMessageType", &itch_reader::FileReader::currentMessageType);
}

