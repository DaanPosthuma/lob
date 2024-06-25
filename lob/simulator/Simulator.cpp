#include "Simulator.h"

#include <optional>
#include <print>

namespace {
    
inline auto toString(std::chrono::nanoseconds timestamp) {
  auto const hours = std::chrono::duration_cast<std::chrono::hours>(timestamp);
  auto const minutes = std::chrono::duration_cast<std::chrono::minutes>(timestamp - hours);
  auto const seconds = std::chrono::duration_cast<std::chrono::seconds>(timestamp - hours - minutes);
  auto const millis = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - hours - minutes - seconds);
  auto const micros = std::chrono::duration_cast<std::chrono::microseconds>(timestamp - hours - minutes - seconds - millis);
  auto const nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp - hours - minutes - seconds - millis - micros);

  std::ostringstream oss;
  oss << std::setw(2) << std::setfill('0') << hours.count() << ":"
      << std::setw(2) << std::setfill('0') << minutes.count() << ":"
      << std::setw(2) << std::setfill('0') << seconds.count() << "."
      << std::setw(3) << std::setfill('0') << millis.count() << "."
      << std::setw(3) << std::setfill('0') << micros.count() << "."
      << std::setw(3) << std::setfill('0') << nanos.count();

  return oss.str();
}
}

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
    //std::print("{} (M): ", toString(marketDataTimestamp));
    mNextMarketDataEvent.second();
    mNextMarketDataEvent = mRequestMarketDataEvent();
    return marketDataTimestamp;
  } else {
    //std::print("{} (S):  ", toString(*strategyTimestamp));
    mNextStrategyEvents.top().second();
    mNextStrategyEvents.pop();
    return *strategyTimestamp;
  }
}
