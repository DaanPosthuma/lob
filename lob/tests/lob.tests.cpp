#include <gtest/gtest.h>
#include <lob/lob.h>

using namespace std::string_literals;

namespace {


TEST(LOB, WalkingTheBook) {
  auto book = lob::LimitOrderBook<4>();

  book.addOrder(lob::Direction::Sell, 100, 231400);
  book.addOrder(lob::Direction::Sell, 100, 231400);
  book.addOrder(lob::Direction::Sell, 50, 231300);
  book.addOrder(lob::Direction::Sell, 100, 231300);
  book.addOrder(lob::Direction::Sell, 100, 231200);
  book.addOrder(lob::Direction::Sell, 50, 231200);
  book.addOrder(lob::Direction::Sell, 100, 231200);

  book.addOrder(lob::Direction::Buy, 100, 230900);
  book.addOrder(lob::Direction::Buy, 100, 230900);
  book.addOrder(lob::Direction::Buy, 50, 230800);
  book.addOrder(lob::Direction::Buy, 100, 230800);
  book.addOrder(lob::Direction::Buy, 50, 230800);
  book.addOrder(lob::Direction::Buy, 100, 230700);
  book.addOrder(lob::Direction::Buy, 100, 230700);

  ASSERT_EQ(static_cast<double>(book.bid()), 23.09);
  ASSERT_EQ(static_cast<double>(book.ask()), 23.12);
  ASSERT_EQ(static_cast<double>(book.bidDepth()), 200);
  ASSERT_EQ(static_cast<double>(book.askDepth()), 250);

  std::cout << "bid: " << book.bid() << std::endl;
  std::cout << "ask: " << book.ask() << std::endl;

  std::cout << "bid depth: " << book.bidDepth() << std::endl;
  std::cout << "ask depth: " << book.askDepth() << std::endl;

  std::cout << book << std::endl;

  /*book.PlaceMarketOrder(lob::Direction::Sell, 270);
  book.PlaceMarketOrder(lob::Direction::Buy, 560);*/
}

}  // namespace
