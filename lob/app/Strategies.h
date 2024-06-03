#pragma once

#include <iostream>

namespace strategies {

template <class T>
concept Strategy = requires(T a) {
  { a.onUpdate() };
};

template <class BookT>
class TrivialStrategy {
 public:
  TrivialStrategy(BookT& book) : mBook(book) {}

  void onUpdate() noexcept {
    if (mCount++ % 10000 == 0) {
      if (!mBook.hasBids() || !mBook.hasAsks()) return;

      std::cout << mBook.bid() << " (" << mBook.bidDepth() << ") " << mBook.ask() << " (" << mBook.askDepth() << ")" << std::endl;
    }
  }

 private:
  BookT& mBook;
  int mCount = 0;
};

/*template <class BookT>
void RunSingleStockStrategyLoop(Strategy, BookT& book) {
}*/

}  // namespace strategies
