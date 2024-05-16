#pragma once

#include <string>
#include "__generator.hpp"

namespace itch_reader {
  void read(std::string const& filename);
  std::generator<int> fibonacci(int n);
}

