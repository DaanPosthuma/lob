#pragma once

#include <atomic>
#include <optional>
#include <print>
#include <string_view>
#include <vector>

namespace logger {

struct LogMessage {
  int timestamp = 0;
  std::string msg = {};
};

class Queue {
 public:
  Queue(size_t size = 1024) : mSize(size), mMessages(mSize) {}

  void push(int timestamp, std::string msg) {
    auto const writeIdx = mask(mNextWrite++);
    auto& [done, slot] = mMessages[writeIdx];
    if (done) {
      done = false;
      std::println("Buffer overflow! writeIdx: {}, mNextWrite: {}, mNextRead: {}, size: {}", writeIdx, mNextWrite.load(), mNextRead, mSize);
    }
    slot.timestamp = timestamp;
    slot.msg = std::move(msg);
    done = true;
  }

  std::optional<LogMessage> pop() {
    if (mNextRead == mNextWrite) return {};
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

  void log(std::string msg) {
    static int logCount = 0;
    mQueue.push(logCount++, std::move(msg));
  }

 private:
  Queue& mQueue;
};

void LogLoopCout(Queue& queue, std::atomic<bool> const& running) {
  while (running) {
  }
}

}  // namespace logger
