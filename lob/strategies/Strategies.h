#pragma once

#include <algorithm>
#include <atomic>
#include <iostream>
#include <limits>
#include <numeric>

#include "StrategyDiagnostics.h"

namespace strategies {

template <class TopOfBookBufferT>
class StrategyBase {
 public:
  StrategyDiagnostics const& loop(this auto& self, std::atomic<bool>& running, TopOfBookBufferT const& topOfBookBuffer, size_t& bufferReadIdx) noexcept {
    while (running.load()) {
      for (int i = 0; i != 10000; ++i) {
        self.onUpdate(topOfBookBuffer, bufferReadIdx);
      }
    }
    return self.diagnostics();
  }

  [[nodiscard]] StrategyDiagnostics const& diagnostics() const noexcept {
    return mDiagnostics;
  }

  [[nodiscard]] StrategyDiagnostics& diagnostics() noexcept {
    return mDiagnostics;
  }

 private:
  StrategyDiagnostics mDiagnostics = {};
};

template <class TopOfBookBufferT>
class TrivialStrategy : private StrategyBase<TopOfBookBufferT> {
 public:
  TrivialStrategy() {}

  using StrategyBase<TopOfBookBufferT>::diagnostics;
  using StrategyBase<TopOfBookBufferT>::loop;

  void onUpdate(TopOfBookBufferT const& topOfBookBuffer, size_t& bufferReadIdx) noexcept {
    auto const [updates, m, M] = topOfBookBuffer.read(bufferReadIdx);

    if (updates.empty()) return;

    diagnostics().bufferLoad(bufferReadIdx, m, M);

    // std::cout << "onUpdate timestamp: " << toString(timestamp) << ", num updates: " << updates.size() << ", m: " << m << ", M: " << M << '\n';

    // auto top = updates.back();
    for (auto top : updates) {
      auto const now = std::chrono::high_resolution_clock::now();
      auto const update = updates.back().first;
      diagnostics().addLag(now - update);
      // std::cout << toString(timestamp) << ": " << top.bid << " (" << top.bidDepth << ") " << top.ask << " (" << top.askDepth << ")\n";
      diagnostics().addObs(static_cast<double>(top.second.bid), static_cast<double>(top.second.ask));
    }
    bufferReadIdx = M + 1;
  }
};

}  // namespace strategies
