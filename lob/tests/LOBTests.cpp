#include <gtest/gtest.h>
#include <lob/lob.h>
#include <lob/itch_reader.h>
#include <unordered_map>

#include <lob/bufferedreader.h>
#include <lob/itch.h>
#include <lob/order_book.h>

namespace {

  static sprice_t mksigned(price_t price, BUY_SELL buy)
  {
    assert(MKPRIMITIVE(price) < std::numeric_limits<int32_t>::max());
    auto ret = MKPRIMITIVE(price);
    if (BUY_SELL::SELL == buy) ret = -ret;
    return sprice_t(ret);
  }

  TEST(LOBTests, ItchReaderTest) {
    
    size_t countAddOrder = 0;

    auto books = std::vector<lob::LimitOrderBook<4>>(10000);
    // order_book::oid_map.max_load_factor(0.5);
    order_book::oid_map.reserve(order_id_t(184118975 * 2));  // the first number
                                                             // is the empirically
                                                             // largest oid seen.
                                                             // multiply by 2 for
                                                             // good measure

    auto add_order = [&](uint64_t oid, uint16_t posid, uint32_t price, char bs, uint32_t qty){
      ++countAddOrder;
      //books[posid].PlaceLimitOrder(bs == 'B' ? lob::Direction::Buy : lob::Direction::Sell, qty, price);
      order_book::add_order(order_id_t(oid), book_id_t(posid),
                              mksigned(price_t(price), BUY_SELL(bs)), qty_t(qty));
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

