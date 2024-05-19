#include <gtest/gtest.h>
#include <lob/lob.h>

using namespace std::string_literals;

namespace {


TEST(Filter, Filter) {
  /*auto c0 = Condition(StringCondition("BuySell == B"));
  auto c1 = Condition(StringCondition("OrderType == A"));
  auto c2 = Condition(StringCondition("Something == 3"));

  std::cout << "c0: " << c0 << std::endl;
  std::cout << "c1: " << c1 << std::endl;

  std::cout << "c0 and c1: " << AndCondition{c0, c1} << std::endl;
  std::cout << "c0 or c1: " << OrCondition{c0, c1} << std::endl;
  std::cout << "c0 && (c1 || c2): " << AndCondition{c0, OrCondition{c1, c2}} << std::endl;*/
}

TEST(LOB, WalkingTheBook) {
  /*auto book = lob::LimitOrderBook<4>();

  book.PlaceLimitOrder(lob::Direction::Sell, 100, 231400);
  book.PlaceLimitOrder(lob::Direction::Sell, 100, 231400);
  book.PlaceLimitOrder(lob::Direction::Sell, 50, 231300);
  book.PlaceLimitOrder(lob::Direction::Sell, 100, 231300);
  book.PlaceLimitOrder(lob::Direction::Sell, 100, 231200);
  book.PlaceLimitOrder(lob::Direction::Sell, 50, 231200);
  book.PlaceLimitOrder(lob::Direction::Sell, 100, 231200);

  book.PlaceLimitOrder(lob::Direction::Buy, 100, 230900);
  book.PlaceLimitOrder(lob::Direction::Buy, 100, 230900);
  book.PlaceLimitOrder(lob::Direction::Buy, 50, 230800);
  book.PlaceLimitOrder(lob::Direction::Buy, 100, 230800);
  book.PlaceLimitOrder(lob::Direction::Buy, 50, 230800);
  book.PlaceLimitOrder(lob::Direction::Buy, 100, 230700);
  book.PlaceLimitOrder(lob::Direction::Buy, 100, 230700);

  book.PlaceMarketOrder(lob::Direction::Sell, 270);
  book.PlaceMarketOrder(lob::Direction::Buy, 560);*/
}

}  // namespace
