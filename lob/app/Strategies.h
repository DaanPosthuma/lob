#pragma once

#include <iostream>

namespace strategies {

template <class BookT>
class TrivialStrategy {
 public:
  TrivialStrategy(BookT& book) : mBook(book) {}

  void onUpdate(auto timestamp) noexcept {
    auto const top = mBook.top();
    if (top != mPreviousTop) {
      std::cout << toString(timestamp) << ": " << top.bid << " (" << top.bidDepth << ") " << top.ask << " (" << top.askDepth << ")\n";
      mPreviousTop = top;
    }
  }

 private:
  BookT& mBook;
  BookT::TopOfBook mPreviousTop;
};

}  // namespace strategies
