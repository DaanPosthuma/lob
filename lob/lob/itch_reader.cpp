#include "itch_reader.h"
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

void itch_reader::read() {
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
