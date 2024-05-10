#include <gtest/gtest.h>
#include <lob/lob.h>
#include <lob/itch_reader.h>
#include <unordered_map>


namespace {

  TEST(LOBTests, ItchReaderTest) {
    
    size_t countAddOrder = 0;

    auto books = std::vector<lob::LimitOrderBook<4>>(10000);

    auto add_order = [&](uint64_t oid, uint16_t posid, uint32_t price, char bs, uint32_t qty){
      ++countAddOrder;
      books[posid].PlaceLimitOrder(bs == 'B' ? lob::Direction::Buy : lob::Direction::Sell, qty, price);
    };


    itch_reader::read(add_order);
    
    std::cout << "count add order: " << countAddOrder << std::endl;
    std::cout << "num books: " << books.size() << std::endl;
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

