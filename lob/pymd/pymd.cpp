#include <lob/lob.h>
#include <md/BinaryDataReader.h>
#include <md/MappedFile.h>
#include <md/Symbols.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <simulator/ItchBooksManager.h>
#include <simulator/Simulator.h>
#include <simulator/functions.h>
#include <strategies/Strategies.h>

#include <memory>
#include <print>

namespace py = pybind11;

namespace {

class LOBReference {
 public:
  LOBReference(lob::LimitOrderBook& lob) : mLob(lob) {}

 private:
  std::reference_wrapper<lob::LimitOrderBook> mLob;
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

void testStrategies(
    md::BinaryDataReader reader,
    std::unordered_map<int, std::vector<std::reference_wrapper<strategies::TestStrategy>>> strategiesBySymbolId,
    unsigned int numIters) {
  auto bmgr = simulator::ItchBooksManager{};
  auto simulator = simulator::Simulator{[&] { return simulator::getNextMarketDataEvent(reader, bmgr); }};
  
  auto prevTopById = boost::unordered_map<int, lob::LimitOrderBook::TopOfBook>{};
  for (auto const& [symbolId, strategies] : strategiesBySymbolId) {
    prevTopById[symbolId] = bmgr.bookById(symbolId).top();
  }
  
  for (int i : std::views::iota(0u, numIters)) {
    
    if (PyErr_CheckSignals() != 0)
      throw py::error_already_set();
    
    auto const timestamp = simulator.step();

    for (auto const& [symbolId, strategies] : strategiesBySymbolId) {
      auto& prevTop = prevTopById.at(symbolId);
      auto const& currTop = bmgr.bookById(symbolId).top();
      if (prevTop != currTop) {
        for (auto& strategy : strategies) {
          strategy.get().onUpdate(timestamp, currTop);
        }
        prevTop = currTop;
      }
    }
  }
}

auto getTopOfBookData(
    md::BinaryDataReader reader,
    int const& symbolId,
    unsigned int numIters) {
  auto bmgr = simulator::ItchBooksManager{};
  auto simulator = simulator::Simulator{[&] { return simulator::getNextMarketDataEvent(reader, bmgr); }};
  
  auto prevTop = bmgr.bookById(symbolId).top();

  auto timestamps = std::vector<std::chrono::nanoseconds>{};
  auto bids = std::vector<double>{};
  auto asks = std::vector<double>{};
  
  for (int i : std::views::iota(0u, numIters)) {
    
    if (PyErr_CheckSignals() != 0)
      throw py::error_already_set();
    
    auto const timestamp = simulator.step();
    
    auto const& currTop = bmgr.bookById(symbolId).top();
    if (prevTop != currTop) {
      timestamps.push_back(timestamp);
      bids.push_back(static_cast<double>(currTop.bid));
      asks.push_back(static_cast<double>(currTop.ask));
      prevTop = currTop;
    }
    
  }

  return std::tuple{timestamps, bids, asks};
}

}  // namespace

PYBIND11_MODULE(pymd, m) {
  m.doc() = "trading simulator";

  py::class_<md::MappedFile>(m, "MappedFile")
      .def(py::init<std::string>())
      .def_property_readonly("size", &md::MappedFile::size)
      .def("__str__", [](md::MappedFile const& f) { return std::format("<MappedFile(size={}) at {}>", formatBytes(f.size()), static_cast<void const*>(&f)); });

  py::class_<md::BinaryDataReader>(m, "BinaryDataReader")
      .def(py::init([](md::MappedFile const& file) { return md::BinaryDataReader(file.data(), file.size()); }))
      .def_property_readonly("remaining", &md::BinaryDataReader::remaining)
      .def("reset", &md::BinaryDataReader::reset, py::arg("offset") = 0)
      .def("curr", &md::BinaryDataReader::curr)
      .def("__str__", [](md::BinaryDataReader const& r) { return std::format("<BinaryDataReader(remaining={}) at {}>", formatBytes(r.remaining()), static_cast<void const*>(&r)); });

  py::class_<md::utils::Symbols>(m, "Symbols")
      .def(py::init<md::BinaryDataReader&>())
      .def_property_readonly("count", &md::utils::Symbols::count)
      .def("byName", &md::utils::Symbols::byName)
      .def("byId", &md::utils::Symbols::byId)
      .def("__iter__", [](md::utils::Symbols const& s) { return py::make_iterator(s.begin(), s.end()); }, py::keep_alive<0, 1>())
      .def("__str__", [](md::utils::Symbols const& s) { return std::format("<Symbols(count={}) at {}>", s.count(), static_cast<void const*>(&s)); });

  py::class_<LOBReference>(m, "LOBReference")
      .def("__str__", [](LOBReference const& lob) { return std::format("<LOBReference at {}>", static_cast<void const*>(&lob)); });

  py::class_<simulator::ItchBooksManager>(m, "ItchBooksManager")
      .def(py::init<>())
      .def("bookById", [](simulator::ItchBooksManager& bmgr, int id) { return LOBReference(bmgr.bookById(id)); }, py::keep_alive<0, 1>())
      .def("optIn", &simulator::ItchBooksManager::optIn)
      .def("__str__", [](simulator::ItchBooksManager const& bmgr) { return std::format("<ItchBooksManager at {}>", static_cast<void const*>(&bmgr)); });

  py::class_<simulator::OMS>(m, "OMS")
      .def(py::init<>())
      .def("__str__", [](simulator::OMS const& oms) { return std::format("<OMS(...) at {}>", static_cast<void const*>(&oms)); });

  py::class_<strategies::StrategyDiagnostics>(m, "StrategyDiagnostics")
      .def("__str__", [](strategies::StrategyDiagnostics const& sd) { return std::format("<StrategyDiagnostics at {}>", static_cast<void const*>(&sd)); })
      .def("print", &strategies::StrategyDiagnostics::print);

  py::class_<strategies::TestStrategy>(m, "TestStrategy")
      .def(py::init<simulator::OMS&, int, int>())
      .def("__str__", [](strategies::TestStrategy const& s) { return std::format("<TestStrategy at {}>", static_cast<void const*>(&s)); })
      .def_property_readonly("diagnostics", [](strategies::TestStrategy const& s) { return s.diagnostics(); });

  m.def("testStrategies", &testStrategies);
  m.def("getTopOfBookData", &getTopOfBookData);

}
