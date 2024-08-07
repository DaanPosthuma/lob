﻿#include <md/BinaryDataReader.h>
#include <md/MappedFile.h>
#include <md/Symbols.h>
#include <simulator/functions.h>


#include <chrono>
#include <print>

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
  
  auto const file = getTestFile();
  auto const maxNumIters = 10000000;

  auto reader = md::BinaryDataReader(file.data(), file.size());
  auto const symbols = md::utils::Symbols(reader);
  std::println("Loaded {} symbols", symbols.count());

  auto marketStart = reader.curr();

  {
    reader.reset(marketStart);
    std::println("Single thread:");
    auto const start = std::chrono::high_resolution_clock::now();
    simulator::runTest(reader, symbols, maxNumIters, true);
    auto const end = std::chrono::high_resolution_clock::now();
    std::println("Time: {}.\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
  }

  { 
    reader.reset(marketStart);
    auto reader = md::BinaryDataReader(file.data(), file.size());
    std::println("Multi threaded:");
    auto const start = std::chrono::high_resolution_clock::now();
    simulator::runTest(reader, symbols, maxNumIters, false);
    auto const end = std::chrono::high_resolution_clock::now();
    std::println("Time: {}.\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
  }

}
