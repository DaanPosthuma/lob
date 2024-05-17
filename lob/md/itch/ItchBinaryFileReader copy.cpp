#include "ItchBinaryFileReader.h"
#include "itch.h"
#include <iostream>
#include <stdexcept>
#include <utility>
#include <chrono>

using namespace std::string_literals;

namespace {

  template <MessageType __code>
  itch_message<__code> read_itch_message(itch_reader::BinaryDataReader& buf)
  {
    uint16_t const msglen = be16toh(*(uint16_t *)buf.get(0));
    buf.advance(2);
    assert(msglen == netlen<__code>);

    itch_message<__code> ret = itch_message<__code>::parse(buf.get(0));
    buf.advance(netlen<__code>);
    return ret;
  }

  auto read_itch_message(itch_reader::BinaryDataReader reader)
  {
    
    if (reader.remaining() < 3) throw std::runtime_error("not enough data");
    auto const msgtype = MessageType(*reader.get(2));

    auto ret = std::map<std::string, std::variant<std::string, int>>();
    ret["MessageType"] = std::string(1, (char)msgtype);

    switch (msgtype) {
      case MessageType::SYSEVENT: { 
        auto const msg = read_itch_message<MessageType::SYSEVENT>(reader);
        break;
      }
      case MessageType::STOCK_DIRECTORY: { 
        auto const msg = read_itch_message<MessageType::STOCK_DIRECTORY>(reader);
        break;
      }
      case MessageType::TRADING_ACTION: { 
        auto const msg = read_itch_message<MessageType::TRADING_ACTION>(reader);
        break;
      }
      case MessageType::REG_SHO_RESTRICT: { 
        auto const msg = read_itch_message<MessageType::REG_SHO_RESTRICT>(reader);
        break;
      }
      case MessageType::MPID_POSITION: { 
        auto const msg = read_itch_message<MessageType::MPID_POSITION>(reader);
        break;
      }
      case MessageType::MWCB_DECLINE: { 
        auto const msg = read_itch_message<MessageType::MWCB_DECLINE>(reader);
        break;
      }
      case MessageType::MWCB_STATUS: { 
        auto const msg = read_itch_message<MessageType::MWCB_STATUS>(reader);
        break;
      }
      case MessageType::IPO_QUOTE_UPDATE: { 
        auto const msg = read_itch_message<MessageType::IPO_QUOTE_UPDATE>(reader);
        break;
      }
      case MessageType::TRADE: { 
        auto const msg = read_itch_message<MessageType::TRADE>(reader);
        break;
      }
      case MessageType::CROSS_TRADE: { 
        auto const msg = read_itch_message<MessageType::CROSS_TRADE>(reader);
        break;
      }
      case MessageType::BROKEN_TRADE: { 
        auto const msg = read_itch_message<MessageType::BROKEN_TRADE>(reader);
        break;
      }
      case MessageType::NET_ORDER_IMBALANCE: { 
        auto const msg = read_itch_message<MessageType::NET_ORDER_IMBALANCE>(reader);
        break;
      }
      case MessageType::RETAIL_PRICE_IMPROVEMENT: { 
        auto const msg = read_itch_message<MessageType::RETAIL_PRICE_IMPROVEMENT>(reader);
        break;
      }
      case MessageType::PROCESS_LULD_AUCTION_COLLAR_MESSAGE: { 
        auto const msg = read_itch_message<MessageType::PROCESS_LULD_AUCTION_COLLAR_MESSAGE>(reader);
        break;
      }
      case MessageType::ADD_ORDER: { 
        auto const msg = read_itch_message<MessageType::ADD_ORDER>(reader);
        ret["Timestamp"] = std::to_string(std::to_underlying(msg.timestamp));
        ret["OrderID"] = std::to_string(std::to_underlying(msg.oid));
        ret["Price"] = (int)std::to_underlying(msg.price);
        ret["Qty"] = (int)std::to_underlying(msg.qty);
        ret["StockLocate"] = (int)msg.stock_locate;
        ret["BuySell"] = std::string() + std::to_underlying(msg.buy);
        break;
      }
      case MessageType::ADD_ORDER_MPID: { 
        auto const msg = read_itch_message<MessageType::ADD_ORDER_MPID>(reader);
        ret["Timestamp"] = std::to_string(std::to_underlying(msg.add_msg.timestamp));
        ret["OrderID"] = std::to_string(std::to_underlying(msg.add_msg.oid));
        ret["Price"] = (int)std::to_underlying(msg.add_msg.price);
        ret["Qty"] = (int)std::to_underlying(msg.add_msg.qty);
        ret["StockLocate"] = (int)msg.add_msg.stock_locate;
        ret["BuySell"] = std::string() + std::to_underlying(msg.add_msg.buy);
        break;
      }
      case MessageType::EXECUTE_ORDER: { 
        auto const msg = read_itch_message<MessageType::EXECUTE_ORDER>(reader);
        break;
      }
      case MessageType::EXECUTE_ORDER_WITH_PRICE: { 
        auto const msg = read_itch_message<MessageType::EXECUTE_ORDER_WITH_PRICE>(reader);
        break;
      }
      case MessageType::REDUCE_ORDER: { 
        auto const msg = read_itch_message<MessageType::REDUCE_ORDER>(reader);
        break;
      }
      case MessageType::DELETE_ORDER: { 
        auto const msg = read_itch_message<MessageType::DELETE_ORDER>(reader);
        break;
      }
      case MessageType::REPLACE_ORDER: { 
        auto const msg = read_itch_message<MessageType::REPLACE_ORDER>(reader);
        break;
      }
      default: throw std::runtime_error("Unknown message: "s + (char)msgtype);
    }
    return ret;
  }

}

#define DO_CASE(__itch_t)               \
  case (__itch_t): {                    \
    read_itch_message<__itch_t>(reader); \
    break;                              \
  }

itch_reader::FileReader::FileReader(std::string const& filename) : mFile(filename), mDataReader(mFile.data(), mFile.size()) {
  if(!mFile.is_open()) throw std::runtime_error("Could not load file");
}

std::map<std::string, std::variant<std::string, int>> itch_reader::FileReader::currentMessage() const {
  return read_itch_message(mDataReader);
}

char itch_reader::FileReader::currentMessageType() const {
  if (mDataReader.remaining() < 3) return 0;
  return (char)MessageType(*mDataReader.get(2));
}

bool itch_reader::FileReader::next() {
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
  return mDataReader.remaining() >= 3;
}

void itch_reader::readFullFile(std::string const& filename) {

  std::cout << "Loading " << filename << std::endl;

  boost::iostreams::mapped_file_source file(filename);

  if(!file.is_open()) {
    std::cout << "Could not load file" << std::endl;
    return;
  }

  // Get pointer to the data
  std::cout << "file size: " << file.size() << std::endl;

  auto messageCounts = std::vector<size_t>(256, 0);

  auto reader = itch_reader::BinaryDataReader(file.data(), file.size());

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


