#include "Simulator.h"

#include <optional>
#include <print>

simulator::Simulator::Simulator(std::function<EventT()> requestMarketDataEvent)
    : mRequestMarketDataEvent(std::move(requestMarketDataEvent)),
      mNextMarketDataEvent(mRequestMarketDataEvent()) {}

void simulator::Simulator::addSimulationEvent(EventT event) {
  mNextStrategyEvents.emplace(std::move(event));
}

simulator::Simulator::TimestampT simulator::Simulator::step() {
  auto const marketDataTimestamp = mNextMarketDataEvent.first;
  auto strategyTimestamp = mNextStrategyEvents.empty() ? std::optional<TimestampT>{} : mNextStrategyEvents.top().first;

  if (!strategyTimestamp || marketDataTimestamp < *strategyTimestamp) {
    std::print("{} (market data): ", marketDataTimestamp);
    mNextMarketDataEvent.second();
    mNextMarketDataEvent = mRequestMarketDataEvent();
    return marketDataTimestamp;
  } else {
    std::print("{} (simulation):  ", *strategyTimestamp);
    mNextStrategyEvents.top().second();
    mNextStrategyEvents.pop();
    return *strategyTimestamp;
  }
}
