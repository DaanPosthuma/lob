#pragma once

#include <algorithm>
#include <iostream>
#include <limits>

namespace strategies {

struct StrategyDiagnostics {
  double cumBid = {};
  double cumAsk = {};
  double minBid = std::numeric_limits<double>::max();
  double maxBid = std::numeric_limits<double>::min();
  double minAsk = std::numeric_limits<double>::max();
  double maxAsk = std::numeric_limits<double>::min();
  long numObs = {};
  size_t numBufferOverflows = 0;
  size_t numUpdatesMissed = 0;
  size_t maxBufferSize = 0;

  void addObs(double b, double a) {
    cumBid += b;
    cumAsk += a;
    numObs++;

    if (b > 0) {
      minBid = std::min(minBid, b);
      maxBid = std::max(maxBid, b);
    }

    if (a > 0) {
      minAsk = std::min(minAsk, a);
      maxAsk = std::max(maxAsk, a);
    }
  }

  void bufferLoad(size_t readIdx, size_t m, size_t M) {
    auto misses = m - readIdx;
    numBufferOverflows += misses > 0 ? 1 : 0;
    numUpdatesMissed += m - readIdx;
    maxBufferSize = std::max(maxBufferSize, M - m + 1);
  }

  void print() const {
    std::cout << "Num obs: " << numObs << std::endl;
    std::cout << "Avg bid: " << cumBid / numObs << std::endl;
    std::cout << "Avg ask: " << cumAsk / numObs << std::endl;
    std::cout << "Min/max bid: " << minBid << " " << maxBid << std::endl;
    std::cout << "Min/max ask: " << minAsk << " " << maxAsk << std::endl;

    std::cout << "Buffer overflows: " << numBufferOverflows << std::endl;
    std::cout << "Updates missed: " << numUpdatesMissed << std::endl;
    std::cout << "Max buffer size: " << maxBufferSize << std::endl;
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

  void onUpdate(auto timestamp) noexcept {
    auto [updates, m, M] = mTopOfBookBuffer.read(mBufferReadIdx);

    if (updates.empty()) return;

    mDiagnostics.bufferLoad(mBufferReadIdx, m, M);

    // std::cout << "onUpdate timestamp: " << toString(timestamp) << ", num updates: " << updates.size() << ", m: " << m << ", M: " << M << '\n';

    for (auto top : updates) {
      // std::cout << toString(timestamp) << ": " << top.bid << " (" << top.bidDepth << ") " << top.ask << " (" << top.askDepth << ")\n";
      mDiagnostics.addObs(static_cast<double>(top.bid), static_cast<double>(top.ask));
    }
    mBufferReadIdx = M + 1;
  }

  auto const& getDiagnostics() const {
    return mDiagnostics;
  }

 private:
  typename TopOfBookBufferT::DataT mPreviousTop = {};
  std::function<void(md::itch::types::timestamp_t)> mF = {};

  TopOfBookBufferT const& mTopOfBookBuffer;
  size_t mBufferReadIdx = 0;

  StrategyDiagnostics mDiagnostics = {};
};

}  // namespace strategies
