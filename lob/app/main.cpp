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
    if (from > std::numeric_limits<ToT>::max())
      throw std::runtime_error(std::string("Cannot cast ") + std::to_string(from) + " from " + typeid(FromT).name() + " to " + typeid(ToT).name());
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

auto toString(md::itch::types::timestamp_t timestamp) {
  auto const hours = std::chrono::duration_cast<std::chrono::hours>(timestamp);
  auto const minutes = std::chrono::duration_cast<std::chrono::minutes>(timestamp - hours);
  auto const seconds = std::chrono::duration_cast<std::chrono::seconds>(timestamp - hours - minutes);
  auto const millis = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - hours - minutes - seconds);
  auto const micros = std::chrono::duration_cast<std::chrono::microseconds>(timestamp - hours - minutes - seconds - millis);
  auto const nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp - hours - minutes - seconds - millis - micros);

  std::ostringstream oss;
  oss << std::setw(2) << std::setfill('0') << hours.count() << ":"
      << std::setw(2) << std::setfill('0') << minutes.count() << ":"
      << std::setw(2) << std::setfill('0') << seconds.count() << "."
      << std::setw(3) << std::setfill('0') << millis.count() << "."
      << std::setw(3) << std::setfill('0') << micros.count() << "."
      << std::setw(3) << std::setfill('0') << nanos.count();

  return oss.str();
}

}  // namespace

template <class LobT>
void f() try {
  std::cout << "Loading test file..." << std::endl;

  auto const file = getTestFile();
  auto reader = md::BinaryDataReader(file.data(), file.size());

  auto const [stockLocateMap, numSymbols] = getStockLocateMap(reader);
  std::cout << "Loaded " << numSymbols << " symbols" << std::endl;

  auto timings = std::array<std::pair<size_t, std::chrono::nanoseconds>, 256>{};

  boost::unordered_map<int, LobT> books;

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
        // std::cout << toString(msg.timestamp) << " Added order " << (int)msg.oid << " to book " << msg.stock_locate << std::endl;
        break;
      }
      case md::itch::messages::MessageType::ADD_ORDER_MPID: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::ADD_ORDER_MPID>(reader);
        books[msg.add_msg.stock_locate].addOrder(toLobType(msg.add_msg.oid), toLobType(msg.add_msg.buy), toLobType(msg.add_msg.qty), toLobType(msg.add_msg.price));
        // std::cout << toString(msg.add_msg.timestamp) << " Added (MPID) order " << (int)msg.add_msg.oid << " to book " << msg.add_msg.stock_locate << std::endl;
        break;
      }
      case md::itch::messages::MessageType::REPLACE_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::REPLACE_ORDER>(reader);
        // std::cout << toString(msg.timestamp) << " Todo: replace order " << (int)msg.oid << " with " << (int)msg.new_order_id << std::endl;
        break;
      }
      case md::itch::messages::MessageType::REDUCE_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::REDUCE_ORDER>(reader);
        // std::cout << toString(msg.timestamp) << " Todo: reduce order " << (int)msg.oid << std::endl;
        break;
      }
      case md::itch::messages::MessageType::EXECUTE_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::EXECUTE_ORDER>(reader);
        // std::cout << toString(msg.timestamp) << " Todo: execute order " << (int)msg.oid << std::endl;
        break;
      }
      case md::itch::messages::MessageType::EXECUTE_ORDER_WITH_PRICE: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::EXECUTE_ORDER_WITH_PRICE>(reader);
        // std::cout << toString(msg.exec.timestamp) << " Todo: execute order " << (int)msg.exec.oid << " with price " << std::endl;
        break;
      }
      case md::itch::messages::MessageType::DELETE_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::DELETE_ORDER>(reader);
        auto const orderId = toLobType(msg.oid);
        if (books[msg.stock_locate].deleteOrder(orderId)) {
          //std::cout << toString(msg.timestamp) << " Deleted order " << (int)msg.oid << " from book " << msg.stock_locate << "!" << std::endl;
        } else {
          //std::cout << toString(msg.timestamp) << " Could not delete order " << (int)msg.oid << " from book " << msg.stock_locate << std::endl;
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
} catch (std::exception const& ex) {
  std::cout << "Exception: " << ex.what() << std::endl;
} catch (...) {
  std::cout << "Unknown exception" << std::endl;
}

int main() {
  f<lob::LimitOrderBook<4>>();
  f<lob::LimitOrderBook<4>>();
  f<lob::LimitOrderBookWithLocks<4>>();
  f<lob::LimitOrderBookWithLocks<4>>();
}
