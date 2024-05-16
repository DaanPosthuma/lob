#include <pybind11/pybind11.h>
#include <md/itch_reader.h>
#include <string>

namespace {

  void readFile(std::string& filename) {
    itch_reader::read(filename);
  }

  /*int fib(int n) {
    auto g = itch_reader::fibonacci(n);
    auto it = g.begin();
    for (int i=0; i != n; ++i) {
      ++it;
    }
    return *it;
  }*/

}

PYBIND11_MODULE(pymd, m) {
  m.doc() = "market data plugin";
  m.def("read", &readFile); 
}

