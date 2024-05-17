#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <md/MappedFile.h>
#include <string>
#include <variant>

namespace py = pybind11;
using namespace std::string_literals;

namespace {
    std::map<std::string, std::variant<std::string, int>> test() {
        std::map<std::string, std::variant<std::string, int>> ret;
        ret["test"] = "test"s;
        ret["three"] = 3;
        return ret;
    }
}

PYBIND11_MODULE(pymd, m) {
  
  m.doc() = "market data plugin";
  
  m.def("test", &test);

  py::class_<md::MappedFile>(m, "MappedFile")
    .def(py::init<std::string const&>());
    /*.def("next", &itch_reader::FileReader::next)
    .def("reset", &itch_reader::FileReader::reset)
    .def("currentMessage", &itch_reader::FileReader::currentMessage)
    .def("currentMessageType", &itch_reader::FileReader::currentMessageType)*/
}

