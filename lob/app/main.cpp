#include <logger/Logger.h>
#include <md/BinaryDataReader.h>
#include <md/MappedFile.h>
#include <md/Symbols.h>
#include <simulator/functions.h>

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

}  // namespace

int main() {
  auto logger = logging::Logger{};

  auto loggerPtr = &logger;
  //loggerPtr = nullptr;

  auto const file = getTestFile();
  auto const maxNumIters = 10000000;

  auto reader = md::BinaryDataReader(file.data(), file.size());
  auto const symbols = md::utils::Symbols(reader);
  if (loggerPtr) loggerPtr->log("Loaded {} symbols", symbols.count());

  auto marketStart = reader.curr();

  {
    reader.reset(marketStart);
    if (loggerPtr) loggerPtr->log("Single thread:");
    auto const start = std::chrono::high_resolution_clock::now();
    simulator::runTest(reader, symbols, maxNumIters, true, loggerPtr);
    auto const end = std::chrono::high_resolution_clock::now();
    if (loggerPtr) loggerPtr->log("Time: {}.\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
  }

  {
    reader.reset(marketStart);
    auto reader = md::BinaryDataReader(file.data(), file.size());
    if (loggerPtr) loggerPtr->log("Multi threaded:");
    auto const start = std::chrono::high_resolution_clock::now();
    simulator::runTest(reader, symbols, maxNumIters, false, loggerPtr);
    auto const end = std::chrono::high_resolution_clock::now();
    if (loggerPtr) loggerPtr->log("Time: {}.\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
  }
}
