#pragma once

#include <array>
#include <atomic>
#include <cassert>
#include <deque>

// single producer, multiple consumers ring buffer
template <class T, size_t N>
class RingBuffer {
 public:
  void push(T item) noexcept {
    mData[mSize.load() % N] = item;
    ++mSize;
  }

  [[nodiscard]] auto readWithAsyncF(size_t idx, std::function<void()> const& f) const {
    struct R {
      std::deque<T> data = {};
      size_t m = {};
      size_t M = {};
    };

    R ret;

    auto const s0 = mSize.load();

    f();

    if (idx >= s0) return ret;

    auto const m0 = std::max(idx, s0 > N ? s0 - N : 0);
    auto const M0 = s0 - 1;

    auto const m0m = m0 % N;
    auto const M0m = M0 % N;
    if (m0m <= M0m) {
      std::ranges::copy(mData.data() + m0m, mData.data() + M0m + 1, std::back_inserter(ret.data));
    } else {
      std::ranges::copy(mData.data() + m0m, mData.data() + N, std::back_inserter(ret.data));
      std::ranges::copy(mData.data(), mData.data() + M0m + 1, std::back_inserter(ret.data));
    }

    // some of the data might be invalid if producer wrote over front of data after mSize.load() above, 
    // so we'll discard this potentially invalid data and add new data
    auto const s1 = mSize.load();
    auto const m1 = std::max(idx, s1 > N ? s1 - N : 0);
    auto const M1 = s1 - 1;

    for (int i = M0 + 1; i <= M1; ++i) {
      ret.data.push_back(mData[i % N]);
    }

    for (int i = m0; i != m1; ++i) {
      ret.data.pop_front();
    }

    assert(ret.data.size() == M1 - m1 + 1);
    ret.m = m1;
    ret.M = M1;
    return ret;
  }

  [[nodiscard]] auto read(size_t idx) const {
    return readWithAsyncF(idx, [](){});
  }

 private:
  std::array<T, N> mData = {};
  std::atomic<size_t> mSize = 0;
};
