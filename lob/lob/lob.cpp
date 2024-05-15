#include "lob.h"
#include "itch.h"
#include <iostream>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <print>
#include <utility>
#include <chrono>

using namespace std::string_literals;

namespace {

  class Reader {
  public:
    Reader(char const* data, size_t len) : mData(data), mLen(len){}

    char const* get(size_t n) const { return mData + mCurr + n; }
    void advance(size_t n) { mCurr += n; }
    size_t remaining() const { return mLen - mCurr; }

  private:
    char const* mData;
    size_t mLen;
    size_t mCurr = 0;
    
  };

  template <MessageType __code>
  itch_message<__code> read_itch_message(Reader& reader)
  {
    uint16_t const msglen = be16toh(*(uint16_t *)reader.get(0));
    reader.advance(2);
    assert(msglen == netlen<__code>);

    itch_message<__code> ret = itch_message<__code>::parse(reader.get(0));
    reader.advance(netlen<__code>);
    return ret;
  }

}

#define DO_CASE(__itch_t)               \
  case (__itch_t): {                    \
    read_itch_message<__itch_t>(reader); \
    break;                              \
  }

void lob::test() {
  std::cout << "lob::test()";

  auto const filename = "C:\\dev\\VS\\lob\\01302019.NASDAQ_ITCH50"s;

  std::print("Loading {}\n", filename);

  boost::iostreams::mapped_file_source file(filename);

  if(!file.is_open()) {
    std::print("Could not load file");
    return;
  }

  // Get pointer to the data
  std::print("file size: {}\n", file.size());

  auto messageCounts = std::vector<size_t>(256, 0);

  auto reader = Reader(file.data(), file.size());

  auto const tStart = std::chrono::steady_clock::now();
  size_t numMessages = 0;

  while (reader.remaining() > 3) {
    MessageType const msgtype = MessageType(*reader.get(2));
    ++messageCounts[(int)msgtype];
    ++numMessages;
    //std::print("msgtype: {}\n", (char)msgtype);
    switch (msgtype) {
      DO_CASE(MessageType::SYSEVENT);
      DO_CASE(MessageType::STOCK_DIRECTORY);
      DO_CASE(MessageType::TRADING_ACTION);
      DO_CASE(MessageType::REG_SHO_RESTRICT);
      DO_CASE(MessageType::MPID_POSITION);
      DO_CASE(MessageType::MWCB_DECLINE);
      DO_CASE(MessageType::MWCB_STATUS);
      DO_CASE(MessageType::IPO_QUOTE_UPDATE);
      DO_CASE(MessageType::TRADE);
      DO_CASE(MessageType::CROSS_TRADE);
      DO_CASE(MessageType::BROKEN_TRADE);
      DO_CASE(MessageType::NET_ORDER_IMBALANCE);
      DO_CASE(MessageType::RETAIL_PRICE_IMPROVEMENT);
      DO_CASE(MessageType::PROCESS_LULD_AUCTION_COLLAR_MESSAGE);

      case (MessageType::ADD_ORDER): {
        auto const pkt = read_itch_message<MessageType::ADD_ORDER>(reader);
        //assert(uint64_t(pkt.oid) < uint64_t(std::numeric_limits<int32_t>::max()));
        break;
      }
      case (MessageType::ADD_ORDER_MPID): {
        auto const pkt = read_itch_message<MessageType::ADD_ORDER_MPID>(reader);
        break;
      }
      case (MessageType::EXECUTE_ORDER): {
        auto const pkt = read_itch_message<MessageType::EXECUTE_ORDER>(reader);
        break;
      }
      case (MessageType::EXECUTE_ORDER_WITH_PRICE): {
        auto const pkt = read_itch_message<MessageType::EXECUTE_ORDER_WITH_PRICE>(reader);
        break;
      }
      case (MessageType::REDUCE_ORDER): {
        auto const pkt = read_itch_message<MessageType::REDUCE_ORDER>(reader);
        break;
      }
      case (MessageType::DELETE_ORDER): {
        auto const pkt = read_itch_message<MessageType::DELETE_ORDER>(reader);
        break;
      }
      case (MessageType::REPLACE_ORDER): {
        auto const pkt = read_itch_message<MessageType::REPLACE_ORDER>(reader);
        break;
      }
      default: {
        std::print("Unknown message: {}\n", (char)msgtype);
        return;
      }
    }
  }

  auto const tEnd = std::chrono::steady_clock::now();

  file.close();

  for (char c='A'; c <= 'Z'; ++c) {
    if (messageCounts[c]) {
      std::println("{}: {}", c, messageCounts[c]);
    }
  }
  
  size_t nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(tEnd - tStart).count();
  std::println("{} messages in {} nanos , {} nanos per message", numMessages, nanos, nanos / (double)numMessages);

}

/*void lob::test() {
  std::cout << "lob::test()";

  buf_t buf(1024);
  buf.fd = STDIN_FILENO;
  std::chrono::steady_clock::time_point start;
  size_t npkts = 0;
#define BUILD_BOOK 1
#if !BUILD_BOOK
  size_t nadds(0);
  uint64_t maxoid(0);
#else
  // order_book::oid_map.max_load_factor(0.5);
  order_book::oid_map.reserve(order_id_t(184118975 * 2));  // the first number
                                                           // is the empirically
                                                           // largest oid seen.
                                                           // multiply by 2 for
                                                           // good measure
#endif
  printf("%lu\n", sizeof(order_book) * order_book::MAX_BOOKS);
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
#if BUILD_BOOK
        order_book::add_order(order_id_t(pkt.oid), book_id_t(pkt.stock_locate),
                              mksigned(pkt.price, pkt.buy), pkt.qty);
#else
        int64_t oid = int64_t(pkt.oid);
        maxoid = maxoid > uint64_t(pkt.oid) ? maxoid : uint64_t(pkt.oid);
        // printf("oid:%lu, nadds:%lu, npkts:%lu, %lu, %lu, %f, %f\n", oid,
        // nadds, npkts, maxoid, oid - (int64_t)npkts, oid / (double)nadds, oid
        // / (double)npkts);
        ++nadds;
#endif
        break;
      }
      case (itch_t::ADD_ORDER_MPID): {
        auto const pkt = PROCESS<itch_t::ADD_ORDER_MPID>::read_from(&buf);
#if BUILD_BOOK
        order_book::add_order(
            order_id_t(pkt.add_msg.oid), book_id_t(pkt.add_msg.stock_locate),
            mksigned(pkt.add_msg.price, pkt.add_msg.buy), pkt.add_msg.qty);
#else
        ++nadds;
#endif
        break;
      }
      case (itch_t::EXECUTE_ORDER): {
        auto const pkt = PROCESS<itch_t::EXECUTE_ORDER>::read_from(&buf);
#if BUILD_BOOK
        order_book::execute_order(order_id_t(pkt.oid), pkt.qty);
#endif
        break;
      }
      case (itch_t::EXECUTE_ORDER_WITH_PRICE): {
        auto const pkt =
            PROCESS<itch_t::EXECUTE_ORDER_WITH_PRICE>::read_from(&buf);
#if BUILD_BOOK
        order_book::execute_order(order_id_t(pkt.exec.oid), pkt.exec.qty);
#endif
        break;
      }
      case (itch_t::REDUCE_ORDER): {
        auto const pkt = PROCESS<itch_t::REDUCE_ORDER>::read_from(&buf);
#if BUILD_BOOK
        order_book::cancel_order(order_id_t(pkt.oid), pkt.qty);
#endif
        break;
      }
      case (itch_t::DELETE_ORDER): {
        auto const pkt = PROCESS<itch_t::DELETE_ORDER>::read_from(&buf);
#if BUILD_BOOK
        order_book::delete_order(order_id_t(pkt.oid));
#endif
        break;
      }
      case (itch_t::REPLACE_ORDER): {
        auto const pkt = PROCESS<itch_t::REPLACE_ORDER>::read_from(&buf);
#if BUILD_BOOK
        order_book::replace_order(order_id_t(pkt.oid),
                                  order_id_t(pkt.new_order_id), pkt.new_qty,
                                  mksigned(pkt.new_price, BUY_SELL::BUY));
#endif
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
#if !BUILD_BOOK
  printf("%lu adds\n", nadds);
  printf("maxoid %lu\n", maxoid);
#endif
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  size_t nanos =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("%lu packets in %lu nanos , %.2f nanos per packet \n", npkts, nanos,
         nanos / (double)npkts);
}
*/