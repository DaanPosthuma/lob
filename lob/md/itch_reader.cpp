#include "itch_reader.h"
#include "itch.h"
#include <iostream>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <stdexcept>
#include <utility>
#include <chrono>
//#include <coroutine>
//#include "__generator.hpp"

using namespace std::string_literals;

namespace {

  class DataReader {
  public:
    constexpr DataReader(char const* data, size_t len) noexcept : mData(data), mLen(len){}

    char const* get(size_t n) const noexcept { return mData + mCurr + n; }
    void advance(size_t n) noexcept { mCurr += n; }
    size_t remaining() const noexcept { return mLen - mCurr; }

  private:
    char const* mData;
    size_t mLen;
    size_t mCurr = 0;
    
  };

  template <MessageType __code>
  itch_message<__code> read_itch_message(DataReader& buf)
  {
    uint16_t const msglen = be16toh(*(uint16_t *)buf.get(0));
    buf.advance(2);
    assert(msglen == netlen<__code>);

    itch_message<__code> ret = itch_message<__code>::parse(buf.get(0));
    buf.advance(netlen<__code>);
    return ret;
  }


}

#define DO_CASE(__itch_t)               \
  case (__itch_t): {                    \
    read_itch_message<__itch_t>(reader); \
    break;                              \
  }

class itch_reader::FileReader::Impl {
  public:
    explicit Impl(std::string const& filename) : mFile(filename), mDataReader(mFile.data(), mFile.size()) {
      if(!mFile.is_open()) throw std::runtime_error("Could not load file");
    }

    char currentMessageType() const {
      if (mDataReader.remaining() < 3) return 0;
      return (char)MessageType(*mDataReader.get(2));
    }

    bool next() {
      if (mDataReader.remaining() < 3) return false;
      auto& reader = mDataReader;
      auto const msgtype = MessageType(*mDataReader.get(2));

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
        DO_CASE(MessageType::ADD_ORDER);
        DO_CASE(MessageType::ADD_ORDER_MPID);
        DO_CASE(MessageType::EXECUTE_ORDER);
        DO_CASE(MessageType::EXECUTE_ORDER_WITH_PRICE);
        DO_CASE(MessageType::REDUCE_ORDER);
        DO_CASE(MessageType::DELETE_ORDER);
        DO_CASE(MessageType::REPLACE_ORDER);
        default: throw std::runtime_error("Unknown message: "s + (char)msgtype);
      }
      return true;
    }
    
  private:
    boost::iostreams::mapped_file_source mFile;
    DataReader mDataReader;
};

itch_reader::FileReader::FileReader(std::string const& filename) : mImpl(new Impl(filename)) {}
char itch_reader::FileReader::currentMessageType() const { return mImpl->currentMessageType(); }
bool itch_reader::FileReader::next() { return mImpl->next(); }
itch_reader::FileReader::~FileReader() = default;


void itch_reader::read(std::string const& filename) {

  std::cout << "Loading " << filename << std::endl;

  boost::iostreams::mapped_file_source file(filename);

  if(!file.is_open()) {
    std::cout << "Could not load file" << std::endl;
    return;
  }

  // Get pointer to the data
  std::cout << "file size: " << file.size() << std::endl;

  auto messageCounts = std::vector<size_t>(256, 0);

  auto reader = DataReader(file.data(), file.size());

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
        std::cout << "Unknown message: " << (char)msgtype << std::endl;
        return;
      }
    }
  }

  auto const tEnd = std::chrono::steady_clock::now();

  file.close();

  for (char c='A'; c <= 'Z'; ++c) {
    if (messageCounts[c]) {
      std::cout << c << ": " << messageCounts[c] << std::endl;
    }
  }
  
  size_t nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(tEnd - tStart).count();
  std::cout << numMessages << " messages in " << nanos << " nanos , " << nanos / (double)numMessages << " nanos per message" << std::endl;

}


