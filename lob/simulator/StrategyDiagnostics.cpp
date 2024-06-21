#include "StrategyDiagnostics.h"

#include <algorithm>
#include <format>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <numeric>

namespace std::chrono {

void to_json(nlohmann::json& j, std::chrono::nanoseconds ns) {
  j = ns.count();
}

}  // namespace std::chrono

void strategies::StrategyDiagnostics::print() const {
  std::cout << "Num obs: " << bids.size() << ", " << asks.size() << std::endl;
  std::cout << "Avg bid: " << std::accumulate(bids.begin(), bids.end(), 0.0) / bids.size() << std::endl;
  std::cout << "Avg ask: " << std::accumulate(asks.begin(), asks.end(), 0.0) / asks.size() << std::endl;
  auto [bidsMin, bidsMax] = std::ranges::minmax_element(bids);
  auto [asksMin, asksMax] = std::ranges::minmax_element(asks);
  std::cout << "Min/max bid: " << *bidsMin << " " << *bidsMax << std::endl;
  std::cout << "Min/max ask: " << *asksMin << " " << *asksMax << std::endl;

  std::cout << "Buffer overflows: " << numBufferOverflows << std::endl;
  std::cout << "Updates missed: " << numUpdatesMissed << std::endl;
  std::cout << "Max buffer size: " << maxBufferSize << std::endl;

  auto [lagsMin, lagsMax] = std::ranges::minmax_element(lags);
  auto lagsAvg = std::accumulate(lags.begin(), lags.end(), std::chrono::nanoseconds(0), [](auto accum, auto lag) { return accum + lag; }) / lags.size();

  std::cout << "Average lag: " << lagsAvg << std::endl;
  std::cout << "Min/max lag: " << *lagsMin << " " << *lagsMax << std::endl;

  std::cout << std::endl;
}

void strategies::StrategyDiagnostics::save(std::string const& filename) const {

  nlohmann::json j;
  j["lags"] = lags;
  j["bids"] = bids;
  j["asks"] = asks;

  auto file = std::ofstream(filename);
  if (!file.is_open()) throw std::runtime_error(std::format("Could not open output file '{}'", filename));
  file << j.dump();

}
