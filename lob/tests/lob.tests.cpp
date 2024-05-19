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

  auto const id0 = book.addOrder(lob::Direction::Buy, 100, 230900);
  auto const id1 = book.addOrder(lob::Direction::Buy, 100, 230900);
  book.addOrder(lob::Direction::Buy, 50, 230800);
  book.addOrder(lob::Direction::Buy, 100, 230800);
  book.addOrder(lob::Direction::Buy, 50, 230800);
  book.addOrder(lob::Direction::Buy, 100, 230700);
  book.addOrder(lob::Direction::Buy, 100, 230700);

  ASSERT_EQ(static_cast<int>(book.bid()), 230900);
  ASSERT_EQ(static_cast<int>(book.ask()), 231200);
  ASSERT_EQ(book.bidDepth(), 200);
  ASSERT_EQ(book.askDepth(), 250);

  /*std::cout << "bid: " << book.bid() << std::endl;
  std::cout << "ask: " << book.ask() << std::endl;

  std::cout << "bid depth: " << book.bidDepth() << std::endl;
  std::cout << "ask depth: " << book.askDepth() << std::endl;

  std::cout << book << std::endl;

  std::cout << "deleting order " << id << std::endl;*/
  book.deleteOrder(id0);

  ASSERT_EQ(static_cast<int>(book.bid()), 230900);
  ASSERT_EQ(static_cast<int>(book.ask()), 231200);
  ASSERT_EQ(book.bidDepth(), 100);
  ASSERT_EQ(book.askDepth(), 250);

  book.deleteOrder(id1);

  ASSERT_EQ(static_cast<int>(book.bid()), 230800);
  ASSERT_EQ(static_cast<int>(book.ask()), 231200);
  ASSERT_EQ(book.bidDepth(), 200);
  ASSERT_EQ(book.askDepth(), 250);

  //std::cout << book << std::endl;

  /*book.PlaceMarketOrder(lob::Direction::Sell, 270);
  book.PlaceMarketOrder(lob::Direction::Buy, 560);*/
}

}  // namespace
