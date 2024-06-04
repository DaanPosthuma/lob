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
#include "Strategies.h"

static_assert(strategies::Strategy<strategies::TrivialStrategy<lob::LimitOrderBook<4>>>);

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

auto processMessages(md::BinaryDataReader& reader, auto const& addOrder, auto const& deleteOrder, auto const& onUpdate, int maxCount) {
  
  auto timings = std::array<std::pair<size_t, std::chrono::nanoseconds>, 256>{};
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
        addOrder(msg.timestamp, msg.stock_locate, msg.oid, msg.buy, msg.qty, msg.price);
        break;
      }
      case md::itch::messages::MessageType::ADD_ORDER_MPID: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::ADD_ORDER_MPID>(reader);
        addOrder(msg.add_msg.timestamp, msg.add_msg.stock_locate, msg.add_msg.oid, msg.add_msg.buy, msg.add_msg.qty, msg.add_msg.price);
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
        deleteOrder(msg.timestamp, msg.stock_locate, msg.oid);
        break;
      }
      default:
        md::itch::skipCurrentMessage(reader);
    }

    auto const end = std::chrono::high_resolution_clock::now();
    auto& [count, nanos] = timings[(int)currentMessageType];
    ++count;
    nanos += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    onUpdate();
  }
  return timings;
}

}  // namespace

template <class LobT>
void f() try {
  std::cout << "Loading test file..." << std::endl;

  auto const file = getTestFile();
  auto reader = md::BinaryDataReader(file.data(), file.size());

  auto const symbols = md::utils::Symbols(reader);

  std::cout << "Loaded " << symbols.count() << " symbols" << std::endl;

  boost::unordered_map<int, LobT> books;
  
  auto strategy = [&] {
    auto const id = symbols.byName("QQQ");
    auto& book = books[id];
    return strategies::TrivialStrategy(book);
  }();

  auto addOrder = [&books](auto timestamp, auto stock_locate, auto oid, auto buy, auto qty, auto price) {
    books[stock_locate].addOrder(toLobType(oid), toLobType(buy), toLobType(qty), toLobType(price));
    std::cout << toString(timestamp) << " Added order " << (int)oid << " to book " << stock_locate << std::endl;
  };

  auto deleteOrder = [&books](auto timestamp, auto stock_locate, auto oid) {
    auto const orderId = toLobType(oid);
    if (books[stock_locate].deleteOrder(orderId)) {
      // std::cout << toString(msg.timestamp) << " Deleted order " << (int)msg.oid << " from book " << msg.stock_locate << "!" << std::endl;
    } else {
      // std::cout << toString(msg.timestamp) << " Could not delete order " << (int)msg.oid << " from book " << msg.stock_locate << std::endl;
    }
  };

  auto onUpdate = [&strategy]() {
    strategy.onUpdate();
  };

  auto const timings = processMessages(reader, addOrder, deleteOrder, onUpdate, maxCount);

  for (char msgType = 'A'; msgType <= 'Z'; ++msgType) {
    auto const [count, nanos] = timings[msgType];
    if (count) {
      std::cout << msgType << ": " << static_cast<double>(nanos.count()) / count << " nanos per message (" << count << " messages)" << std::endl;
    }
  }

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
