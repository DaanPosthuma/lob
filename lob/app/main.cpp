#include <lob/RingBuffer.h>
#include <lob/lob.h>
#include <md/BinaryDataReader.h>
#include <md/MappedFile.h>
#include <md/Symbols.h>
#include <md/itch/MessageReaders.h>
#include <md/itch/TypeConverterss.h>

#include <boost/unordered_map.hpp>
#include <chrono>
#include <exec/inline_scheduler.hpp>
#include <exec/static_thread_pool.hpp>
#include <iostream>
#include <stdexec/execution.hpp>
#include <utility>

#include "ItchToLobType.h"
#include "PinToCore.h"
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
    -> simulator::Simulator<md::itch::types::timestamp_t>::EventT {
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
    // std::cout << "timestamp: " << toString(timestamp) << ", executing a message" << std::endl;
    f();
  }
}

template <typename TupleT, std::size_t... Is>
void printTupleImp(const TupleT& tp, std::index_sequence<Is...>) {
  (std::get<Is>(tp).print(), ...);
}

template <typename TupleT, std::size_t TupSize = std::tuple_size_v<TupleT>>
void printTuple(const TupleT& tp) {
  printTupleImp(tp, std::make_index_sequence<TupSize>{});
}

}  // namespace

template <bool SingleThreaded>
void f(int numIters) try {
  using LobT = lob::LimitOrderBook<4>;
  // using LobT = lob::LimitOrderBookWithLocks<4>;

  std::cout << "Loading test file..." << std::endl;

  auto const file = getTestFile();
  auto reader = md::BinaryDataReader(file.data(), file.size());

  auto const symbols = md::utils::Symbols(reader);

  std::cout << "Loaded " << symbols.count() << " symbols" << std::endl;

  auto books = boost::unordered_map<int, LobT>{};
  auto topOfBookBuffers = boost::unordered_map<int, RingBuffer<LobT::TopOfBook, 64>>{};

  auto addOrder = [&books, &topOfBookBuffers](auto timestamp, auto stock_locate, auto oid, auto buy, auto qty, auto price) {
    auto& book = books[stock_locate];
    auto before = book.top();
    book.addOrder(toOrderId(oid), toDirection(buy), toInt(qty), toLevel<LobT::Precision>(price));
    if (before != book.top()) {
      topOfBookBuffers[stock_locate].push(book.top());
    }
    // std::cout << toString(timestamp) << " Added order " << (int)oid << " to book " << stock_locate << std::endl;
  };

  auto deleteOrder = [&books, &topOfBookBuffers](auto timestamp, auto stock_locate, auto oid) {
    auto& book = books[stock_locate];
    auto before = book.top();
    if (books[stock_locate].deleteOrder(toOrderId(oid))) {
      // std::cout << toString(msg.timestamp) << " Deleted order " << (int)msg.oid << " from book " << msg.stock_locate << "!" << std::endl;
    } else {
      // std::cout << toString(msg.timestamp) << " Could not delete order " << (int)msg.oid << " from book " << msg.stock_locate << std::endl;
    }
    if (before != book.top()) {
      topOfBookBuffers[stock_locate].push(book.top());
    }
  };

  auto getNextMarketDataEvent = [&reader, &addOrder, &deleteOrder] {
    return ::getNextMarketDataEvent(reader, addOrder, deleteOrder);
  };

  auto simulator = simulator::Simulator<md::itch::types::timestamp_t>{getNextMarketDataEvent};

  using namespace std::chrono_literals;

  auto createStrategy = [&](std::string const& symbolName) {
    auto const id = symbols.byName(symbolName);
    auto const& topOfBookBuffer = topOfBookBuffers[id];
    return strategies::TrivialStrategy(topOfBookBuffer, [](md::itch::types::timestamp_t) {});
  };

  if constexpr (SingleThreaded) {
    auto strategy = createStrategy("QQQ");
    for (int i : std::views::iota(0, numIters)) {
      auto timestamp = simulator.step();
      if (i % 64 == 0) strategy.onUpdate(timestamp);
    }
    std::cout << "Strategy and simulation done:" << std::endl;
    strategy.getDiagnostics().print();
  } else {
    std::atomic<bool> running = true;

    auto const simulatorLoop = [&]() {
      std::this_thread::sleep_for(1s);

      for (int i : std::views::iota(0, numIters)) {
        simulator.step();
      }
      std::cout << "Simulation done." << std::endl;
      running = false;
    };

    auto const strategyLoop = [&](std::string const& symbolName) {
      auto strategy = createStrategy(symbolName);
      while (running) {
        for (int i = 0; i != 10000; ++i) {
          strategy.onUpdate(md::itch::types::timestamp_t(0));
        }
      }
      return strategy.getDiagnostics();
    };

    auto pool = exec::static_thread_pool(5);

    auto work = stdexec::when_all(
        stdexec::schedule(pool.get_scheduler()) | stdexec::then([] { pin_to_core(0); }) | stdexec::then([&] { return strategyLoop("QQQ"); }),
        stdexec::schedule(pool.get_scheduler()) | stdexec::then([] { pin_to_core(1); }) | stdexec::then([&] { return strategyLoop("SPY"); }),
        stdexec::schedule(pool.get_scheduler()) | stdexec::then([] { pin_to_core(2); }) | stdexec::then([&] { return strategyLoop("AMD"); }),
        stdexec::schedule(pool.get_scheduler()) | stdexec::then([] { pin_to_core(3); }) | stdexec::then([&] { return strategyLoop("IWM"); }),
        stdexec::schedule(pool.get_scheduler()) | stdexec::then([] { pin_to_core(4); }) | stdexec::then(simulatorLoop));

    auto diags = stdexec::sync_wait(std::move(work)).value();

    std::cout << "Done!" << std::endl;

    printTuple(diags);
  }

} catch (std::exception const& ex) {
  std::cout << "Exception: " << ex.what() << std::endl;
} catch (...) {
  std::cout << "Unknown exception" << std::endl;
}

int main() {
  auto const numIters = 10000000;
  {
    std::cout << "Single thread:\n";
    auto const start = std::chrono::high_resolution_clock::now();
    f<true>(numIters);
    auto const end = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " millis.\n\n";
  }

  {
    std::cout << "Multi-threaded:\n";
    auto const start = std::chrono::high_resolution_clock::now();
    f<false>(numIters);
    auto const end = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " millis.\n\n";
  }
}
