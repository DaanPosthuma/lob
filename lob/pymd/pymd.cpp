#include <pybind11/pybind11.h>
#include <md/itch_reader.h>

int add(int i, int j) { return i + j; }

int fib(int n) {
  auto g = itch_reader::fibonacci(n);
  auto it = g.begin();
  for (int i=0; i != n; ++i) {
    ++it;
  }
  return *it;
}


PYBIND11_MODULE(pymd, m) {
  m.doc() = "market data plugin";
  m.def("add", &add);
  m.def("fib", &fib); 
}

