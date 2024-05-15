#include "itch_reader.h"
#include <iostream>
#include <chrono>
#include <utility>
#include "bufferedreader.h"
#include "itch.h"
//#include "order_book.h"

template <itch_t __code>
class PROCESS
{
 public:
  static itch_message<__code> read_from(buf_t *__buf)
  {
    uint16_t const msglen = be16toh(*(uint16_t *)__buf->get(0));
    __buf->advance(2);
    assert(msglen == netlen<__code>);

    __buf->ensure(netlen<__code>);
    itch_message<__code> ret = itch_message<__code>::parse(__buf->get(0));
    __buf->advance(netlen<__code>);
    return ret;
  }
};

/*static sprice_t mksigned(price_t price, BUY_SELL buy)
{
  assert(MKPRIMITIVE(price) < std::numeric_limits<int32_t>::max());
  auto ret = MKPRIMITIVE(price);
  if (BUY_SELL::SELL == buy) ret = -ret;
  return sprice_t(ret);
}*/

#define DO_CASE(__itch_t)               \
  case (__itch_t): {                    \
    PROCESS<__itch_t>::read_from(&buf); \
    break;                              \
  }

void itch_reader::read(std::function<void(uint64_t, uint16_t, uint32_t, char, uint32_t)> const& add_order) {
  std::cout << "itch_reader::read";

  buf_t buf(1024);
  buf.fd = STDIN_FILENO;
  std::chrono::steady_clock::time_point start;
  size_t npkts = 0;
  // order_book::oid_map.max_load_factor(0.5);
  /*order_book::oid_map.reserve(order_id_t(184118975 * 2));  // the first number
                                                           // is the empirically
                                                           // largest oid seen.
                                                           // multiply by 2 for
                                                           // good measure*/
  //printf("%lu\n", sizeof(order_book) * order_book::MAX_BOOKS);
  while (is_ok(buf.ensure(3))) {
    if (npkts) ++npkts;
    itch_t const msgtype = itch_t(*buf.get(2));
    switch (msgtype) {
      DO_CASE(itch_t::SYSEVENT);
      DO_CASE(itch_t::STOCK_DIRECTORY);
      DO_CASE(itch_t::TRADING_ACTION);
      DO_CASE(itch_t::REG_SHO_RESTRICT);
      DO_CASE(itch_t::MPID_POSITION);
      DO_CASE(itch_t::MWCB_DECLINE);
      DO_CASE(itch_t::MWCB_STATUS);
      DO_CASE(itch_t::IPO_QUOTE_UPDATE);
      DO_CASE(itch_t::TRADE);
      DO_CASE(itch_t::CROSS_TRADE);
      DO_CASE(itch_t::BROKEN_TRADE);
      DO_CASE(itch_t::NET_ORDER_IMBALANCE);
      DO_CASE(itch_t::RETAIL_PRICE_IMPROVEMENT);
      DO_CASE(itch_t::PROCESS_LULD_AUCTION_COLLAR_MESSAGE);

      case (itch_t::ADD_ORDER): {
        if (!npkts) {
          start = std::chrono::steady_clock::now();
          ++npkts;
        }
        auto const pkt = PROCESS<itch_t::ADD_ORDER>::read_from(&buf);
        assert(uint64_t(pkt.oid) <
               uint64_t(std::numeric_limits<int32_t>::max()));
        //order_book::add_order(order_id_t(pkt.oid), book_id_t(pkt.stock_locate),
        //                      mksigned(pkt.price, pkt.buy), pkt.qty);
	add_order(std::to_underlying(pkt.oid), pkt.stock_locate, std::to_underlying(pkt.price), std::to_underlying(pkt.buy), std::to_underlying(pkt.qty));
        break;
      }
      case (itch_t::ADD_ORDER_MPID): {
        auto const pkt = PROCESS<itch_t::ADD_ORDER_MPID>::read_from(&buf);
        //order_book::add_order(
        //    order_id_t(pkt.add_msg.oid), book_id_t(pkt.add_msg.stock_locate),
        //    mksigned(pkt.add_msg.price, pkt.add_msg.buy), pkt.add_msg.qty);
        break;
      }
      case (itch_t::EXECUTE_ORDER): {
        auto const pkt = PROCESS<itch_t::EXECUTE_ORDER>::read_from(&buf);
        //order_book::execute_order(order_id_t(pkt.oid), pkt.qty);
        break;
      }
      case (itch_t::EXECUTE_ORDER_WITH_PRICE): {
        auto const pkt = PROCESS<itch_t::EXECUTE_ORDER_WITH_PRICE>::read_from(&buf);
        //order_book::execute_order(order_id_t(pkt.exec.oid), pkt.exec.qty);
        break;
      }
      case (itch_t::REDUCE_ORDER): {
        auto const pkt = PROCESS<itch_t::REDUCE_ORDER>::read_from(&buf);
        //order_book::cancel_order(order_id_t(pkt.oid), pkt.qty);
        break;
      }
      case (itch_t::DELETE_ORDER): {
        auto const pkt = PROCESS<itch_t::DELETE_ORDER>::read_from(&buf);
        //order_book::delete_order(order_id_t(pkt.oid));
        break;
      }
      case (itch_t::REPLACE_ORDER): {
        auto const pkt = PROCESS<itch_t::REPLACE_ORDER>::read_from(&buf);
        //order_book::replace_order(order_id_t(pkt.oid),
        //                          order_id_t(pkt.new_order_id), pkt.new_qty,
        //                          mksigned(pkt.new_price, BUY_SELL::BUY));
        // actually it will get re-signed inside. code smell
        break;
      }
      default: {
        printf("Uh oh bad code %d\n", char(msgtype));
        assert(false);
        break;
      }
    }
  }
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  size_t nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("%lu packets in %lu nanos , %.2f nanos per packet \n", npkts, nanos, nanos / (double)npkts);
}

