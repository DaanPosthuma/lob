#include <gtest/gtest.h>
#include <lob/lob.h>
#include <lob/itch_reader.h>
#include <unordered_map>

#include <lob/itch.h>
#include <lob/order_book.h>

namespace {

  TEST(ItchReader, Read) {
    itch_reader::read("C:\\dev\\VS\\lob\\01302019.NASDAQ_ITCH50");
  }

  TEST(LOB, WalkingTheBook) {
    
    auto book = lob::LimitOrderBook<4>();
    
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
    book.PlaceMarketOrder(lob::Direction::Buy, 560);
  }

}

