#include <lob/lob.h>
#include <md/BinaryDataReader.h>
#include <md/MappedFile.h>
#include <md/Symbols.h>
#include <md/itch/MessageReaders.h>
#include <md/itch/TypeConverterss.h>

#include <boost/unordered_map.hpp>
#include <chrono>
#include <iostream>
#include <utility>

#include "ItchToLobType.h"
#include "Simulator.h"
#include "Strategies.h"

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

auto getNextMarketDataEvent(md::BinaryDataReader& reader, auto const& addOrder, auto const& deleteOrder)
    -> std::pair<md::itch::types::timestamp_t, std::function<void()>> {
  while (reader.remaining() >= 3) {
    auto const currentMessageType = md::itch::currentMessageType(reader);
    auto const start = std::chrono::high_resolution_clock::now();
    switch (currentMessageType) {
      case md::itch::messages::MessageType::ADD_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::ADD_ORDER>(reader);
        return {msg.timestamp, [msg, &addOrder] {
                  addOrder(msg.timestamp, msg.stock_locate, msg.oid, msg.buy, msg.qty, msg.price);
                }};
      }
      case md::itch::messages::MessageType::ADD_ORDER_MPID: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::ADD_ORDER_MPID>(reader);
        return {msg.add_msg.timestamp, [msg, &addOrder] {
                  addOrder(msg.add_msg.timestamp, msg.add_msg.stock_locate, msg.add_msg.oid, msg.add_msg.buy, msg.add_msg.qty, msg.add_msg.price);
                }};
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
        return {msg.timestamp, [msg, &deleteOrder] {
                  deleteOrder(msg.timestamp, msg.stock_locate, msg.oid);
                }};
      }
      default:
        md::itch::skipCurrentMessage(reader);
    }
  }
  throw std::runtime_error("end of messages");
}

auto processMessages(md::BinaryDataReader& reader, auto const& addOrder, auto const& deleteOrder, int maxCount) {
  for (size_t msgi = 0; msgi != maxCount; ++msgi) {
    if (reader.remaining() < 3) {
      std::cout << "No more messages" << std::endl;
      break;
    }
    auto [timestamp, f] = getNextMarketDataEvent(reader, addOrder, deleteOrder);
    //std::cout << "timestamp: " << toString(timestamp) << ", executing a message" << std::endl;
    f();
  }
}

}  // namespace

int main() try {
  using LobT = lob::LimitOrderBook<4>;
  // using LobT = lob::LimitOrderBookWithLocks<4>;

  std::cout << "Loading test file..." << std::endl;

  auto const file = getTestFile();
  auto reader = md::BinaryDataReader(file.data(), file.size());

  auto const symbols = md::utils::Symbols(reader);

  std::cout << "Loaded " << symbols.count() << " symbols" << std::endl;

  auto books = boost::unordered_map<int, LobT>{};

  auto strategy = [&] {
    auto const id = symbols.byName("QQQ");
    auto& book = books[id];
    return strategies::TrivialStrategy(book);
  }();

  auto onUpdate = [&strategy](auto timestamp) {
    strategy.onUpdate(timestamp);
  };

  auto addOrder = [&books, &onUpdate](auto timestamp, auto stock_locate, auto oid, auto buy, auto qty, auto price) {
    books[stock_locate].addOrder(toOrderId(oid), toDirection(buy), toInt(qty), toLevel<LobT::Precision>(price));
    // std::cout << toString(timestamp) << " Added order " << (int)oid << " to book " << stock_locate << std::endl;
    onUpdate(timestamp);
  };

  auto deleteOrder = [&books, &onUpdate](auto timestamp, auto stock_locate, auto oid) {
    if (books[stock_locate].deleteOrder(toOrderId(oid))) {
      // std::cout << toString(msg.timestamp) << " Deleted order " << (int)msg.oid << " from book " << msg.stock_locate << "!" << std::endl;
      onUpdate(timestamp);
    } else {
      // std::cout << toString(msg.timestamp) << " Could not delete order " << (int)msg.oid << " from book " << msg.stock_locate << std::endl;
    }
  };

  auto getNextMarketDataEvent = [&reader, &addOrder, &deleteOrder] {
    ::getNextMarketDataEvent(reader, addOrder, deleteOrder);
  };

  auto simulator = simulator::Simulator<md::itch::types::timestamp_t>{};

  processMessages(reader, addOrder, deleteOrder, maxCount);

} catch (std::exception const& ex) {
  std::cout << "Exception: " << ex.what() << std::endl;
} catch (...) {
  std::cout << "Unknown exception" << std::endl;
}
