#include <md/MappedFile.h>
#include <print>
#include <chrono>

namespace {

using namespace std::string_literals;

auto getTestFile() {
#ifdef _WIN32
  auto const filename = "C:\\dev\\VS\\lob\\data\\01302019.NASDAQ_ITCH50"s;
#else
  auto const filename = "/mnt/itch-data/01302019.NASDAQ_ITCH50";
#endif
  return md::MappedFile(filename);
}

} // namespace

int main() {
  auto file = getTestFile();
  
  auto const numIters = 10000000;
  {
    std::println("Single thread:");
    auto const start = std::chrono::high_resolution_clock::now();
    //f<true>(numIters);
    auto const end = std::chrono::high_resolution_clock::now();
    std::println("Time: {}.\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
  }

  {
    std::println("Multi-threaded:");
    auto const start = std::chrono::high_resolution_clock::now();
    //f<false>(numIters);
    auto const end = std::chrono::high_resolution_clock::now();
    std::println("Time: {}.\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
  }
}
