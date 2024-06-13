#pragma once

#include <functional>
#include <optional>
#include <queue>
#include <vector>

namespace simulator {

template <class TimestampT>
class Simulator {
 public:
  using EventT = std::pair<TimestampT, std::function<void()>>;
  explicit Simulator(std::function<EventT()> requestMarketDataEvent)
      : mRequestMarketDataEvent(std::move(requestMarketDataEvent)),
        mNextMarketDataEvent(mRequestMarketDataEvent()) {
  }

  void addSimulationEvent(EventT event) {
    mNextStrategyEvents.emplace(std::move(event));
  }

  auto step() {
    auto const marketDataTimestamp = mNextMarketDataEvent.first;
    auto strategyTimestamp = mNextStrategyEvents.empty() ? std::optional<TimestampT>{} : mNextStrategyEvents.top().first;

    /*std::cout << "Next market data event: " << toString(marketDataTimestamp) << '\n';
    if (strategyTimestamp) std::cout << "Next strategy event:    " << toString(*strategyTimestamp) << '\n';
    else std::cout << "No next strategy event\n";*/

    if (!strategyTimestamp || marketDataTimestamp < *strategyTimestamp) {
      //std::cout << "Processing market data event " << toString(marketDataTimestamp) << '\n';
      mNextMarketDataEvent.second();
      mNextMarketDataEvent = mRequestMarketDataEvent();
      return marketDataTimestamp;
    } else {
      //std::cout << "Processing simulation event " << toString(*strategyTimestamp) << '\n';
      mNextStrategyEvents.top().second();
      mNextStrategyEvents.pop();
      return *strategyTimestamp;
    }
  }

 private:
  struct CompareEventTimestampGreater {
    [[nodiscard]] auto constexpr operator()(EventT const& lhs, EventT const& rhs) const noexcept {
      return lhs.first > rhs.first;
    }
  };

  std::function<EventT()> mRequestMarketDataEvent;
  EventT mNextMarketDataEvent;
  std::priority_queue<EventT, std::vector<EventT>, CompareEventTimestampGreater> mNextStrategyEvents;
};

}  // namespace simulator
