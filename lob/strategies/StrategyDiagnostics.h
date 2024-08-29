#pragma once

#include <chrono>
#include <vector>

namespace strategies {

struct StrategyDiagnostics {
  size_t numBufferOverflows = 0;
  size_t numUpdatesMissed = 0;
  size_t maxBufferSize = 0;
  std::vector<double> bids;
  std::vector<double> asks;
  std::vector<std::chrono::nanoseconds> lags;

  void addLag(std::chrono::nanoseconds lag) {
    lags.push_back(lag);
  }

  void addObs(double b, double a) {
    if (b > 0) bids.push_back(b);
    if (a > 0) asks.push_back(a);
  }

  void bufferLoad(size_t readIdx, size_t m, size_t M) {
    auto misses = m - readIdx;
    numBufferOverflows += misses > 0 ? 1 : 0;
    numUpdatesMissed += m - readIdx;
    maxBufferSize = std::max(maxBufferSize, M - m + 1);
  }

  [[nodiscard]] std::string toString() const noexcept;
  void save(std::string const& filename) const;
};

}  // namespace strategies
