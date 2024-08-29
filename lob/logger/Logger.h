#pragma once

#include <atomic>
#include <functional>
#include <optional>
#include <print>
#include <thread>
#include <vector>

namespace logging {

struct LogMessage {
  int timestamp = 0;
  std::string msg = {};
  bool overflow = false;
};

namespace handlers {
  void print(logging::LogMessage const& msg);
}


class Queue {
 public:
  explicit Queue(size_t size);

  void push(std::string msg) noexcept;
  [[nodiscard]] std::optional<LogMessage> pop() noexcept;

 private:
  [[nodiscard]] size_t constexpr mask(size_t idx) const noexcept;

  size_t mSize;
  std::vector<std::pair<std::atomic<bool>, LogMessage>> mMessages;
  size_t mNextRead = 0;                // single consumer
  std::atomic<size_t> mNextWrite = 0;  // multiple producers
};

using namespace std::chrono_literals;

class Logger {
 public:
  Logger();
  Logger(size_t queueSize, std::function<void(LogMessage const&)> messageHandler, std::chrono::milliseconds sleepDuration);

  Logger(Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(Logger&) = delete;
  Logger& operator=(Logger&&) = delete;

  template <class... Args>
  void log(std::format_string<Args...> fmt, Args&&... args) {
    mQueue.push(std::format(fmt, std::forward<Args>(args)...));
  }

 private:
  Queue mQueue;
  std::jthread mHandler;
};

}  // namespace logging