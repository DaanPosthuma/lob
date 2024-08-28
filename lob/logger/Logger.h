#pragma once

#include <atomic>
#include <functional>
#include <optional>
#include <print>
#include <string_view>
#include <thread>
#include <vector>

namespace logging {

struct LogMessage {
  int timestamp = 0;
  std::string msg = {};
};

class Queue {
 public:
  explicit Queue(size_t size) : mSize(size), mMessages(mSize) {}

  void push(int timestamp, std::string msg) noexcept {
    auto const writeIdx = mask(mNextWrite++);
    auto& [done, slot] = mMessages[writeIdx];
    if (done) {
      done = false;
      //if (recurse) push(timestamp, std::format("Logger::push - Buffer overflow! writeIdx: {}, mNextWrite: {}, mNextRead: {}, size: {}", writeIdx, mNextWrite.load(), mNextRead, mSize), false);
    }
    slot.timestamp = timestamp;
    slot.msg = std::move(msg);
    done = true;
  }

  [[nodiscard]] std::optional<LogMessage> pop() noexcept {
    if (mNextRead == mNextWrite) return {};
    if (mNextWrite - mNextRead > mSize) {
      //if (recurse) push({}, std::format("Logger::pop - Missed {} message(s)", mNextWrite - mNextRead - mSize), false);
      mNextRead = mNextWrite - mSize;
    }
    auto readIdx = mask(mNextRead);
    auto& [done, msg] = mMessages[readIdx];
    if (!done) return {};
    done = false;
    ++mNextRead;
    return msg;
  }

 private:
  auto constexpr mask(size_t idx) const noexcept {
    return idx & (mSize - 1);
  }

  size_t mSize;
  std::vector<std::pair<std::atomic<bool>, LogMessage>> mMessages;
  size_t mNextRead = 0;                // single consumer
  std::atomic<size_t> mNextWrite = 0;  // multiple producers
};

class Logger {
 public:
  Logger(Queue& queue) : mQueue(queue) {}

  template <class... Args>
  void log(std::format_string<Args...> fmt, Args&&... args) {
    mQueue.push(0, std::format(fmt, std::forward<Args>(args)...));
  }

 private:
  Queue& mQueue;
};

class Manager {
 public:
  Manager(size_t queueSize, std::function<void(LogMessage const&)> const& messageHandler, auto sleepDuration)
      : mQueue(queueSize), mLogger(mQueue), mHandler([this, &messageHandler, sleepDuration](std::stop_token stopToken) { loop(stopToken, messageHandler, sleepDuration); }) {}
  [[nodiscard]] Logger& logger() noexcept { return mLogger; }

 private:
  void loop(std::stop_token stopToken, std::function<void(LogMessage const&)> const& logMessageHandler, auto sleepDuration) {
    while (!stopToken.stop_requested()) {
      if (auto msg = mQueue.pop()) {
        logMessageHandler(*msg);
      } else {
        std::this_thread::sleep_for(sleepDuration);
      }
    }
  }

  Queue mQueue;
  Logger mLogger;
  std::jthread mHandler;
};

void printLogMessage(logging::LogMessage const& msg) {
  std::println("{}", msg.msg);
};

#define LOG(...) if (loggerPtr != nullptr) { loggerPtr->log(__VA_ARGS__); }

}  // namespace logging
