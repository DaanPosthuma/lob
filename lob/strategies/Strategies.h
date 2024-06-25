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
        auto const [updates, m, M] = topOfBookBuffer.read(bufferReadIdx);

        if (updates.empty()) continue;

        auto const update = updates.back().first;
        self.diagnostics().bufferLoad(bufferReadIdx, m, M);

        for (auto top : updates) {
          auto const now = std::chrono::high_resolution_clock::now();
          self.diagnostics().addLag(now - update);
          self.diagnostics().addObs(static_cast<double>(top.second.bid), static_cast<double>(top.second.ask));

          self.onUpdate(top);
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

template <class TopOfBookBufferT>
class TrivialStrategy : private StrategyBase<TopOfBookBufferT> {
 public:
  TrivialStrategy() {}

  using StrategyBase<TopOfBookBufferT>::diagnostics;
  using StrategyBase<TopOfBookBufferT>::loop;

  void onUpdate(auto const& top) noexcept {
    
  }
};

}  // namespace strategies
