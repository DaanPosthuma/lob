#pragma once

#include <algorithm>
#include <iostream>
#include <limits>
#include <numeric>

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

  void print() const {
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
    auto lagsAvg = std::accumulate(lags.begin(), lags.end(), std::chrono::nanoseconds(0), [](auto accum, auto lag){ return accum + lag; }) / lags.size();

    std::cout << "Average lag: " << lagsAvg << std::endl;
    std::cout << "Min/max lag: " << *lagsMin << " " << *lagsMax << std::endl;

    std::cout << std::endl;
  }
};

template <class TopOfBookBufferT>
class TrivialStrategy {
 public:
  TrivialStrategy(TopOfBookBufferT const& topOfBookBuffer,
                  std::function<void(md::itch::types::timestamp_t)> f)
      : mTopOfBookBuffer(topOfBookBuffer),
        mF(std::move(f)) {}

  void onUpdate() noexcept {
    
    auto const [updates, m, M] = mTopOfBookBuffer.read(mBufferReadIdx);

    if (updates.empty()) return;

    mDiagnostics.bufferLoad(mBufferReadIdx, m, M);

    // std::cout << "onUpdate timestamp: " << toString(timestamp) << ", num updates: " << updates.size() << ", m: " << m << ", M: " << M << '\n';

    //for (auto top : updates) 
    auto top = updates.back();
    {
      auto const now = std::chrono::high_resolution_clock::now();
      auto const update = updates.back().first;
      //auto lag = std::chrono::duration_cast<std::chrono::nanoseconds>(now - update);
      mDiagnostics.addLag(now - update);
      // std::cout << toString(timestamp) << ": " << top.bid << " (" << top.bidDepth << ") " << top.ask << " (" << top.askDepth << ")\n";
      mDiagnostics.addObs(static_cast<double>(top.second.bid), static_cast<double>(top.second.ask));
      
    }
    mBufferReadIdx = M + 1;
  }

  auto const& getDiagnostics() const {
    return mDiagnostics;
  }

 private:
  typename TopOfBookBufferT::DataT::second_type mPreviousTop = {};
  std::function<void(md::itch::types::timestamp_t)> mF = {};

  TopOfBookBufferT const& mTopOfBookBuffer;
  size_t mBufferReadIdx = 0;

  StrategyDiagnostics mDiagnostics = {};
};

}  // namespace strategies
