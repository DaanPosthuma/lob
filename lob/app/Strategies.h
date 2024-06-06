#pragma once

#include <iostream>

namespace strategies {

template <class BookT>
class TrivialStrategy {
 public:
  TrivialStrategy(BookT& book, std::function<void(md::itch::types::timestamp_t)> f) : mBook(book), mF(std::move(f)) {}

  void onUpdate(auto timestamp) noexcept {
    if (mC % 3 == 0) {
      std::cout << "Strategy iteration " << mC << " triggering some trading event\n";
      mF(timestamp);
    }
    ++mC;

    auto const top = mBook.top();
    if (top != mPreviousTop) {
      std::cout << toString(timestamp) << ": " << top.bid << " (" << top.bidDepth << ") " << top.ask << " (" << top.askDepth << ")\n";
      mPreviousTop = top;
    }
  }

 private:
  BookT& mBook;
  BookT::TopOfBook mPreviousTop = {};
  int mC = 1;
  std::function<void(md::itch::types::timestamp_t)> mF = {};
};

}  // namespace strategies
