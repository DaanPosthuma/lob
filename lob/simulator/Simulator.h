#pragma once

#include <chrono>
#include <functional>
#include <queue>
#include <vector>

namespace simulator {

class Simulator {
 public:
  using TimestampT = std::chrono::nanoseconds;
  using EventT = std::pair<TimestampT, std::function<void()>>;

  explicit Simulator(std::function<EventT()> requestMarketDataEvent);
  void addSimulationEvent(EventT event);
  TimestampT step();

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
