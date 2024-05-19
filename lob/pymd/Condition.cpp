#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

#include "Condition.h"

using namespace std::string_literals;

std::string AndCondition::toString() const {
  auto ret = "("s;
  std::ranges::for_each(mConditions, [first = true, &ret](auto const& condition) mutable { ret += (first ? "" : " && ") + condition.toString(); first = false; });
  ret += ")";
  return ret;
}

std::string OrCondition::toString() const {
  auto ret = "("s;
  std::ranges::for_each(mConditions, [first = true, &ret](auto const& condition) mutable { ret += (first ? "" : " || ") + condition.toString(); first = false; });
  ret += ")";
  return ret;
}

namespace py = pybind11;

PYBIND11_MODULE(pymdf, m) {
  m.doc() = "md filter plugin";

  py::class_<Condition>(m, "Condition")
      .def(py::init([](std::string const& condition){ return Condition(StringCondition(condition)); }))
      .def("toString", &Condition::toString)
      .def(py::self | py::self)
      .def(py::self & py::self);
}
