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

std::string strategies::StrategyDiagnostics::toString() const noexcept {
  std::stringstream ss;
  ss << "Num obs: " << bids.size() << ", " << asks.size() << std::endl;
  
  if (bids.size()) {
    auto [bidsMin, bidsMax] = std::ranges::minmax_element(bids);
    ss << "Avg bid: " << std::accumulate(bids.begin(), bids.end(), 0.0) / bids.size() << std::endl;
    ss << "Min/max bid: " << *bidsMin << " " << *bidsMax << std::endl;
  }

  if (asks.size()) {
    auto [asksMin, asksMax] = std::ranges::minmax_element(asks);
    ss << "Avg ask: " << std::accumulate(asks.begin(), asks.end(), 0.0) / asks.size() << std::endl;
    ss << "Min/max ask: " << *asksMin << " " << *asksMax << std::endl;
  }
  
  ss << "Buffer overflows: " << numBufferOverflows << std::endl;
  ss << "Updates missed: " << numUpdatesMissed << std::endl;
  ss << "Max buffer size: " << maxBufferSize << std::endl;

  if (lags.size()) {
    auto [lagsMin, lagsMax] = std::ranges::minmax_element(lags);
    auto lagsAvg = std::accumulate(lags.begin(), lags.end(), std::chrono::nanoseconds(0), [](auto accum, auto lag) { return accum + lag; }) / lags.size();
    ss << "Average lag: " << lagsAvg << std::endl;
    ss << "Min/max lag: " << *lagsMin << " " << *lagsMax << std::endl;
  }
  return ss.str();
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
