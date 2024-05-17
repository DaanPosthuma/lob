#pragma once

namespace md {

  class BinaryDataReader {
  public:
    constexpr BinaryDataReader(char const* data, size_t len) noexcept : mData(data), mLen(len) {}

    char const* get(size_t n) const noexcept { return mData + mCurr + n; }
    void advance(size_t n) noexcept { mCurr += n; }
    size_t remaining() const noexcept { return mLen - mCurr; }
    void reset(size_t offset = 0) { mCurr = offset; }

  private:
    char const* mData;
    size_t mLen;
    size_t mCurr = 0;
  };

}  // namespace md
