#include <gtest/gtest.h>
#include <lob/lob.h>
#include <unordered_map>


namespace {

  TEST(LOBTests, Test) {
	lob::test();
  }


  TEST(LOBTests, Trivial) {
    
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

