#include <lob/lob.h>
#include <md/BinaryDataReader.h>
#include <md/MappedFile.h>
#include <md/itch/MessageReaders.h>

#include <boost/unordered_map.hpp>
#include <chrono>
#include <iostream>
#include <utility>

namespace {

using namespace std::string_literals;

auto constexpr static maxCount = 10000000;

auto getTestFile() {
#ifdef _WIN32
  auto const filename = "C:\\dev\\VS\\lob\\data\\01302019.NASDAQ_ITCH50"s;
#else
  auto const filename = "/mnt/itch-data/01302019.NASDAQ_ITCH50";
#endif
  return md::MappedFile(filename);
}

auto getStockLocateMap(md::BinaryDataReader& reader) {
  auto map = std::vector<std::string>(std::numeric_limits<uint16_t>::max());
  int count = 0;
  bool preTrading = true;

  while (preTrading) {
    auto const currentMessageType = md::itch::currentMessageType(reader);
    switch (currentMessageType) {
      case md::itch::messages::MessageType::SYSEVENT: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::SYSEVENT>(reader);
        if (msg.eventCode == 'S') preTrading = false;
        break;
      }
      case md::itch::messages::MessageType::STOCK_DIRECTORY: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::STOCK_DIRECTORY>(reader);
        map[msg.stock_locate] = msg.stock;
        ++count;
        break;
      }
      default:
        md::itch::skipCurrentMessage(reader);
    }
  }
  return std::pair(map, count);
}

template <class ToT, class FromT>
void assertCanCastTo(FromT from) {
  if constexpr (!std::is_same<FromT, ToT>::value) {
    if(from > std::numeric_limits<ToT>::max()) 
      throw std::runtime_error(std::string("Cannot cast ") + std::to_string(from) + " from " + typeid(FromT).name() + " to " + typeid(ToT).name());
    //if(from < std::numeric_limits<ToT>::min()) 
    //  throw std::runtime_error(std::string("Cannot cast ") + std::to_string(from) + " from " + typeid(FromT).name() + " to " + typeid(ToT).name());
  }
}

auto toLobType(md::itch::types::oid_t oid) {
  assertCanCastTo<int>(std::to_underlying(oid));
  return lob::OrderId(static_cast<int>(oid));
}

auto toLobType(md::itch::types::BUY_SELL bs) {
  return bs == md::itch::types::BUY_SELL::BUY ? lob::Direction::Buy : lob::Direction::Sell;
}

auto toLobType(md::itch::types::price_t price) {
  assertCanCastTo<int>(std::to_underlying(price));
  return static_cast<int>(price);
}

auto toLobType(md::itch::types::qty_t qty) {
  assertCanCastTo<int>(std::to_underlying(qty));
  return static_cast<int>(qty);
}

}  // namespace

int main() try {

  std::cout << "Loading test file..." << std::endl;

  auto const file = getTestFile();
  auto reader = md::BinaryDataReader(file.data(), file.size());

  auto const [stockLocateMap, numSymbols] = getStockLocateMap(reader);
  std::cout << "Loaded " << numSymbols << " symbols" << std::endl;

  auto timings = std::array<std::pair<size_t, std::chrono::nanoseconds>, 256>{};

  boost::unordered_map<int, lob::LimitOrderBook<4>> books;

  for (size_t msgi = 0; msgi != maxCount; ++msgi) {
    if (reader.remaining() < 3) {
      std::cout << "No more messages (msgi: " << msgi << ")" << std::endl;
      break;
    }
    auto const currentMessageType = md::itch::currentMessageType(reader);
    auto const start = std::chrono::high_resolution_clock::now();
    switch (currentMessageType) {
      case md::itch::messages::MessageType::ADD_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::ADD_ORDER>(reader);
        books[msg.stock_locate].addOrder(toLobType(msg.oid), toLobType(msg.buy), toLobType(msg.qty), toLobType(msg.price));
        break;
      }
      case md::itch::messages::MessageType::ADD_ORDER_MPID: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::ADD_ORDER_MPID>(reader);
        books[msg.add_msg.stock_locate].addOrder(toLobType(msg.add_msg.oid), toLobType(msg.add_msg.buy), toLobType(msg.add_msg.qty), toLobType(msg.add_msg.price));
        break;
      }
      case md::itch::messages::MessageType::REPLACE_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::REPLACE_ORDER>(reader);
        //books[msg.add_msg.stock_locate].addOrder(toLobType(msg.add_msg.oid), toLobType(msg.add_msg.buy), toLobType(msg.add_msg.qty), toLobType(msg.add_msg.price));
        std::cout << "replace order " << (int)msg.oid << " with " << (int)msg.new_order_id << std::endl;
        break;
      }
      case md::itch::messages::MessageType::DELETE_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::DELETE_ORDER>(reader);
        try {
          books[msg.stock_locate].deleteOrder(toLobType(msg.oid));
          std::cout << "Deleted order " << (int)msg.oid << " from book " << msg.stock_locate << "!" << std::endl;
        }
        catch (...) {
          std::cout << "Exception trying to delete order " << (int)msg.oid << " from book " << msg.stock_locate << std::endl;
          //std::cout << books[msg.stock_locate] << std::endl;
          throw;
        }

        break;
      }
      default:
        md::itch::skipCurrentMessage(reader);
    }
    auto const end = std::chrono::high_resolution_clock::now();
    auto& [count, nanos] = timings[(int)currentMessageType];
    ++count;
    nanos += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  }

  for (char msgType = 'A'; msgType <= 'Z'; ++msgType) {
    auto const [count, nanos] = timings[msgType];
    if (count) {
      std::cout << msgType << ": " << static_cast<double>(nanos.count()) / count << " nanos per message (" << count << " messages)" << std::endl;
    }
  }

  /*for (auto const& [symbolId, book] : books) {
    std::cout << symbolId << " (" << stockLocateMap[symbolId] << ")" << std::endl;
    
    if (book.hasBids()) {
      std::cout << "Bid: " << book.bid() << " (" << book.bidDepth() << ")" << std::endl;
    } else {
      std::cout << "No bids" << std::endl;
    }

    if (book.hasAsks()) {
      std::cout << "Ask: " << book.ask() << " (" << book.askDepth() << ")" << std::endl;
    } else {
      std::cout << "No asks" << std::endl;
    }
  }*/
} 
catch(std::exception const& ex) {
  std::cout << "Exception: " << ex.what() << std::endl;
}
catch(...) {
  std::cout << "Unknown exception" << std::endl;
}

