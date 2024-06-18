#pragma once

#include <algorithm>
#include <iostream>
#include <limits>
#include <numeric>

#include "StrategyDiagnostics.h"

namespace strategies {

template <class TopOfBookBufferT>
class TrivialStrategy {
 public:
  TrivialStrategy(TopOfBookBufferT const& topOfBookBuffer)
      : mTopOfBookBuffer(topOfBookBuffer) {}

  void onUpdate() noexcept {
    auto const [updates, m, M] = mTopOfBookBuffer.read(mBufferReadIdx);

    if (updates.empty()) return;

    mDiagnostics.bufferLoad(mBufferReadIdx, m, M);

    // std::cout << "onUpdate timestamp: " << toString(timestamp) << ", num updates: " << updates.size() << ", m: " << m << ", M: " << M << '\n';

    // auto top = updates.back();
    for (auto top : updates) {
      auto const now = std::chrono::high_resolution_clock::now();
      auto const update = updates.back().first;
      mDiagnostics.addLag(now - update);
      // std::cout << toString(timestamp) << ": " << top.bid << " (" << top.bidDepth << ") " << top.ask << " (" << top.askDepth << ")\n";
      mDiagnostics.addObs(static_cast<double>(top.second.bid), static_cast<double>(top.second.ask));
    }
    mBufferReadIdx = M + 1;
  }

  [[nodiscard]] auto const& getDiagnostics() const {
    return mDiagnostics;
  }

 private:
  typename TopOfBookBufferT::DataT::second_type mPreviousTop = {};

  TopOfBookBufferT const& mTopOfBookBuffer;
  size_t mBufferReadIdx = 0;

  StrategyDiagnostics mDiagnostics = {};
};

}  // namespace strategies
