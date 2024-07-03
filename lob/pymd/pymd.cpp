#include <lob/lob.h>
#include <md/BinaryDataReader.h>
#include <md/MappedFile.h>
#include <md/Symbols.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <simulator/Simulator.h>

#include <memory>
#include <print>

namespace {

class LOBReference {
 public:
  LOBReference(lob::LimitOrderBook& lob) : mLob(lob) {}

 private:
  std::reference_wrapper<lob::LimitOrderBook> mLob;
};

class LOBCollection {
 public:
  LOBReference getRefefence(int i) {
    if (!mLobs.contains(i)) throw std::runtime_error(std::format("LOBCollection does not contain index {}", i));
    return mLobs[i];
  }

 private:
  std::unordered_map<int, lob::LimitOrderBook> mLobs;
};

[[nodiscard]] auto formatBytes(size_t bytes) noexcept {
  constexpr static auto suffixes = std::array{"B", "KB", "MB", "GB", "TB", "PB", "EB"};
  size_t s = 0;
  double count = bytes;
  while (count >= 1024 && s < suffixes.size()) {
    s++;
    count /= 1024;
  }

  return std::format("{:.1f}{}", count, suffixes[s]);
}

}  // namespace

namespace py = pybind11;

PYBIND11_MODULE(pymd, m) {
  m.doc() = "trading simulator";

  py::class_<md::MappedFile>(m, "MappedFile")
      .def(py::init<std::string>())
      .def_property_readonly("size", &md::MappedFile::size)
      .def("__str__", [](const md::MappedFile& f) { return std::format("<MappedFile(size={})>", formatBytes(f.size())); });

  py::class_<md::BinaryDataReader>(m, "BinaryDataReader")
      .def(py::init([](md::MappedFile const& file) { return md::BinaryDataReader(file.data(), file.size()); }))
      .def_property_readonly("remaining", &md::BinaryDataReader::remaining)
      .def("reset", &md::BinaryDataReader::reset, py::arg("offset") = 0)
      .def("__str__", [](const md::BinaryDataReader& r) { return std::format("<BinaryDataReader(remaining={})>", formatBytes(r.remaining())); });

  py::class_<md::utils::Symbols>(m, "Symbols")
      .def(py::init<md::BinaryDataReader&>())
      .def_property_readonly("count", &md::utils::Symbols::count)
      .def("byName", &md::utils::Symbols::byName)
      .def("byId", &md::utils::Symbols::byId)
      .def("__iter__", [](md::utils::Symbols const& s) { return py::make_iterator(s.begin(), s.end()); }, py::keep_alive<0, 1>())
      .def("__str__", [](const md::utils::Symbols& s) { return std::format("<Symbols(count={})>", s.count()); });

  py::class_<LOBReference>(m, "LOB")
      .def("__str__", [](const LOBReference& lob) { return std::format("<LOB>"); });

  py::class_<LOBCollection>(m, "LOBCollection")
      .def(py::init<>())
      .def("get", &LOBCollection::getRefefence, py::keep_alive<0, 1>())
      .def("__str__", [](const LOBCollection& c) { return std::format("<LOBCollection>"); });
}
