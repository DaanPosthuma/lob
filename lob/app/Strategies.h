#pragma once

#include <iostream>

namespace strategies {

namespace detail {

struct StrategyDiagnostics {
  double cumBid = {};
  double cumAsk = {};
  double minBid = std::numeric_limits<double>::max();
  double maxBid = std::numeric_limits<double>::min();
  double minAsk = std::numeric_limits<double>::max();
  double maxAsk = std::numeric_limits<double>::min();
  long numObs = {};

  void addObs(double b, double a) {
    cumBid += b;
    cumAsk += a;
    numObs++;

    if (b > 0) {
      minBid = std::min(minBid, b);
      maxBid = std::max(maxBid, b);
    }

    if (a > 0) {
      minAsk = std::min(minAsk, a);
      maxAsk = std::max(maxAsk, a);
    }
  }

  void print() const {
    std::cout << "Num obs: " << numObs << std::endl;
    std::cout << "Avg bid: " << cumBid / numObs << std::endl;
    std::cout << "Avg ask: " << cumAsk / numObs << std::endl;
    std::cout << "Min/max bid: " << minBid << " " << maxBid << std::endl;
    std::cout << "Min/max ask: " << minAsk << " " << maxAsk << std::endl;
  }
};

}  // namespace detail

template <class BookT>
class TrivialStrategy {
 public:
  TrivialStrategy(BookT& book, std::function<void(md::itch::types::timestamp_t)> f) : mBook(book), mF(std::move(f)) {}

  void onUpdate(auto timestamp) noexcept {
    auto const top = mBook.top();
    if (top != mPreviousTop) {
      std::cout << toString(timestamp) << ": " << top.bid << " (" << top.bidDepth << ") " << top.ask << " (" << top.askDepth << ")\n";
      mPreviousTop = top;

      mDiagnostics.addObs(static_cast<double>(top.bid), static_cast<double>(top.ask));
    }
  }

  void printDiagnostics() const {
    mDiagnostics.print();
  }

 private:
  BookT& mBook;
  BookT::TopOfBook mPreviousTop = {};
  std::function<void(md::itch::types::timestamp_t)> mF = {};

  detail::StrategyDiagnostics mDiagnostics = {};
};

}  // namespace strategies
