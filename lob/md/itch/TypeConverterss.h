#pragma once

#include <chrono>
#include <iomanip>
#include <sstream>

#include "types.h"

namespace md::itch::types {

inline auto toString(md::itch::types::timestamp_t timestamp) {
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

}  // namespace md::itch::types
