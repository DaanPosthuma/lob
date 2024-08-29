#pragma once

#include <logger/Logger.h>
#include <simulator/OMS.h>

#include <algorithm>
#include <atomic>
#include <deque>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>

#include "StrategyDiagnostics.h"

namespace strategies {

class StrategyBase {
 public:
  auto const& loop(this auto& self, std::atomic<bool>& running, auto const& topOfBookBuffer, size_t& bufferReadIdx) noexcept {
    while (running.load()) {
      for (int i = 0; i != 10000; ++i) {
        auto const [updates, m, M] = topOfBookBuffer.read(bufferReadIdx);

        if (updates.empty()) continue;

        auto const mostRecentUpdateTimestamp = updates.back().first;
        self.diagnostics().bufferLoad(bufferReadIdx, m, M);

        for (auto update : updates) {
          auto const& [timestamp, top] = update;
          self.diagnostics().addLag(std::chrono::high_resolution_clock::now() - timestamp);

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

template <class T>
class Accumulator {
 public:
  Accumulator(size_t n) : mN(n) {}

  void add(T x) {
    mData.push_back(x);
    mAccum += x;

    if (mData.size() > mN) {
      mAccum -= mData.front();
      mData.pop_front();
    }
  }

  auto accum() const {
    return mAccum;
  }

  auto size() const {
    return mData.size();
  }

  auto avg() const {
    return mAccum / mData.size();
  }

 private:
  T mAccum = {};
  std::deque<T> mData;
  size_t mN;
};

inline auto microprice(auto const& top) {
  auto const vb = top.bidDepth;
  auto const va = top.askDepth;
  auto const pb = static_cast<double>(top.bid);
  auto const pa = static_cast<double>(top.ask);
  return (vb * pa + va * pb) / (vb + va);
}

class TestStrategy : private StrategyBase {
 public:
  TestStrategy(simulator::OMS& oms, int symbolId, size_t k = 100, logging::Logger* logger = nullptr) : mLogger(logger), mAccumP(k), mAccumPSq(k), mSymbolId(symbolId), mOMS(oms) {}

  using StrategyBase::diagnostics;
  using StrategyBase::loop;

  void onUpdate(auto timestamp, auto const& top) noexcept {
    if (static_cast<int>(top.bid) == 0 || static_cast<int>(top.ask) == 0) return;
    auto const price = microprice(top);

    diagnostics().addObs(static_cast<double>(top.bid), static_cast<double>(top.ask));

    mAccumP.add(price);
    mAccumPSq.add(price * price);

    auto const mean = mAccumP.avg();
    auto const variance = mAccumPSq.avg() - mean * mean;
    auto const stdev = std::sqrt(variance);

    auto const qty = 1;

    if (!mPosition) {
      if (static_cast<double>(top.bid) > mean + 2 * stdev) {
        
        if (mLogger) {
          mLogger->log("Bid/ask: {}/{}. micro: {}. Mean: {}, stdev: {}", static_cast<double>(top.bid), static_cast<double>(top.ask), price, mean, stdev);
          mLogger->log("bid > mean + 2 * stdev. {} > {}", static_cast<double>(top.bid), mean + 2 * stdev);
          mLogger->log("selling for {}", static_cast<double>(top.bid));
        }
        mPosition = {static_cast<double>(top.bid), '-'};
        // mOMS.limitOrder(mSymbolId, md::itch::types::BUY_SELL::SELL, qty, mean);
      } else if (static_cast<double>(top.ask) < mean - 2 * stdev) {

        if (mLogger) {
          mLogger->log("Bid/ask: {}/{}. micro: {}. Mean: {}, stdev: {}", static_cast<double>(top.bid), static_cast<double>(top.ask), price, mean, stdev);
          mLogger->log("ask < mean - 2 * stdev. {} > {}", static_cast<double>(top.ask), mean - 2 * stdev);
          mLogger->log("buying for {}", static_cast<double>(top.ask));
        }
        
        mPosition = {static_cast<double>(top.ask), '+'};
        // mOMS.limitOrder(mSymbolId, md::itch::types::BUY_SELL::SELL, qty, mean);
      }
    } else {
      // try to close out position
    }
  }

  Accumulator<double> mAccumP;
  Accumulator<double> mAccumPSq;
  int mSymbolId;
  simulator::OMS& mOMS;
  std::optional<std::tuple<double, char>> mPosition;
  logging::Logger* mLogger;
};

}  // namespace strategies
