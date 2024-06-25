#pragma once

#include <algorithm>
#include <atomic>
#include <iostream>
#include <limits>
#include <numeric>

#include "StrategyDiagnostics.h"

namespace strategies {

class StrategyBase {
 public:
  StrategyDiagnostics const& loop(this auto& self, std::atomic<bool>& running, auto const& topOfBookBuffer, size_t& bufferReadIdx) noexcept {
    while (running.load()) {
      for (int i = 0; i != 10000; ++i) {
        auto const [updates, m, M] = topOfBookBuffer.read(bufferReadIdx);

        if (updates.empty()) continue;

        auto const mostRecentUpdateTimestamp = updates.back().first;
        self.diagnostics().bufferLoad(bufferReadIdx, m, M);

        for (auto update : updates) {
          auto const& [timestamp, top] = update;
          self.diagnostics().addLag(std::chrono::high_resolution_clock::now() - timestamp);
          self.diagnostics().addObs(static_cast<double>(top.bid), static_cast<double>(top.ask));

          self.onUpdate(timestamp, top);
        }
        
        bufferReadIdx = M + 1;
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

class TrivialStrategy : private StrategyBase {
 public:
  TrivialStrategy() {}

  using StrategyBase::diagnostics;
  using StrategyBase::loop;

  void onUpdate(auto timestamp, auto const& top) noexcept {
    
  }
};

}  // namespace strategies
