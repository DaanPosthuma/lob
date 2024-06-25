#include "functions.h"

#include <lob/RingBuffer.h>
#include <lob/lob.h>
#include <md/BinaryDataReader.h>
#include <md/MappedFile.h>
#include <md/Symbols.h>
#include <md/itch/MessageReaders.h>
#include <md/itch/TypeFormatters.h>
#include <strategies/Strategies.h>

#include <boost/unordered_map.hpp>
#include <chrono>
#include <print>
#include <utility>

#include "ItchToLobType.h"
#include "PinToCore.h"
#include "Simulator.h"
#include "TupleMap.h"

#include <stdexec/execution.hpp>
#include <exec/inline_scheduler.hpp>
#include <exec/static_thread_pool.hpp>

namespace {

using namespace std::string_literals;

auto getNextMarketDataEvent(md::BinaryDataReader& reader, auto const& addOrder, auto const& deleteOrder, auto const& reduceOrder, auto const& replaceOrder, auto const& executeOrder)
    -> simulator::Simulator::EventT {
  while (reader.remaining() >= 3) {
    auto const currentMessageType = md::itch::currentMessageType(reader);
    auto const start = std::chrono::high_resolution_clock::now();
    switch (currentMessageType) {
      case md::itch::messages::MessageType::ADD_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::ADD_ORDER>(reader);
        return {msg.timestamp, [msg, &addOrder] {
                  addOrder(msg.stock_locate, msg.oid, msg.buy, msg.qty, msg.price);
                }};
      }
      case md::itch::messages::MessageType::ADD_ORDER_MPID: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::ADD_ORDER_MPID>(reader);
        return {msg.add_msg.timestamp, [msg = msg.add_msg, &addOrder] {
                  addOrder(msg.stock_locate, msg.oid, msg.buy, msg.qty, msg.price);
                }};
      }
      case md::itch::messages::MessageType::REPLACE_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::REPLACE_ORDER>(reader);
        return {msg.timestamp, [msg, &replaceOrder] {
                  replaceOrder(msg.stock_locate, msg.oid, msg.new_order_id, msg.new_qty, msg.new_price);
                }};
      }
      case md::itch::messages::MessageType::REDUCE_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::REDUCE_ORDER>(reader);
        return {msg.timestamp, [msg, &reduceOrder] {
                  reduceOrder(msg.stock_locate, msg.oid, msg.qty);
                }};
        break;
      }
      case md::itch::messages::MessageType::EXECUTE_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::EXECUTE_ORDER>(reader);
        return {msg.timestamp, [msg, &executeOrder] {
                  executeOrder(msg.stock_locate, msg.oid, msg.qty);
                }};
      }
      case md::itch::messages::MessageType::EXECUTE_ORDER_WITH_PRICE: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::EXECUTE_ORDER_WITH_PRICE>(reader);
        return {msg.exec.timestamp, [msg = msg.exec, &executeOrder] {
                  executeOrder(msg.stock_locate, msg.oid, msg.qty);
                }};
      }
      case md::itch::messages::MessageType::DELETE_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::DELETE_ORDER>(reader);
        return {msg.timestamp, [msg, &deleteOrder] {
                  deleteOrder(msg.stock_locate, msg.oid);
                }};
      }
      default:
        md::itch::skipCurrentMessage(reader);
    }
  }
  throw std::runtime_error("end of messages");
}

}  // namespace

void simulator::f(md::BinaryDataReader& reader, int numIters, bool singleThreaded) try {
  using LobT = lob::LimitOrderBook<4>;
  using TopOfBookBuffer = RingBuffer<std::pair<std::chrono::high_resolution_clock::time_point, LobT::TopOfBook>, 64>;

  std::println("Loading test file...");

  auto const symbols = md::utils::Symbols(reader);

  std::println("Loaded {} symbols", symbols.count());

  auto books = boost::unordered_map<int, LobT>{};
  auto topOfBookBuffers = boost::unordered_map<int, TopOfBookBuffer>{};

  topOfBookBuffers[symbols.byName("QQQ")];
  topOfBookBuffers[symbols.byName("SPY")];
  topOfBookBuffers[symbols.byName("AMD")];
  topOfBookBuffers[symbols.byName("IWM")];

  auto addOrder = [&books, &topOfBookBuffers](auto stockLocate, auto oid, auto buy, auto qty, auto price) {
    auto& book = books[stockLocate];
    auto before = book.top();
    book.addOrder(toOrderId(oid), toDirection(buy), toInt(qty), toLevel<LobT::Precision>(price));
    std::println("Added order {}. Size: {}", oid, (int)qty);
    if (before != book.top()) {
      topOfBookBuffers[stockLocate].push({std::chrono::high_resolution_clock::now(), book.top()});
    }
  };

  auto deleteOrder = [&books, &topOfBookBuffers](auto stockLocate, auto oid) {
    auto& book = books[stockLocate];
    auto before = book.top();
    if (books[stockLocate].deleteOrder(toOrderId(oid))) {
      std::println("Deleted order {}", oid);
    } else {
      throw std::runtime_error(std::format("Could not delete order {}", oid));
    }
    if (before != book.top()) {
      topOfBookBuffers[stockLocate].push({std::chrono::high_resolution_clock::now(), book.top()});
    }
  };

  auto replaceOrder = [&books, &topOfBookBuffers](auto stockLocate, auto oid, auto newOid, auto newQty, auto newPrice) {
    auto& book = books[stockLocate];
    auto before = book.top();
    if (books[stockLocate].replaceOrder(toOrderId(oid), toOrderId(newOid), toInt(newQty), toLevel<LobT::Precision>(newPrice))) {
      std::println("Replaced order {} with {}", oid, newOid);
    } else {
      throw std::runtime_error(std::format("Could not replace order {} with {}", oid, newOid));
    }
    if (before != book.top()) {
      topOfBookBuffers[stockLocate].push({std::chrono::high_resolution_clock::now(), book.top()});
    }
  };

  auto reduceOrder = [&books, &topOfBookBuffers](auto stockLocate, auto oid, auto qty) {
    auto& book = books[stockLocate];
    auto before = book.top();
    if (books[stockLocate].reduceOrder(toOrderId(oid), toInt(qty))) {
      std::println("Reduced order {} by {}", oid, (int)qty);
    } else {
      throw std::runtime_error(std::format("Could not reduce order {} in book!!", oid, stockLocate));
    }
    if (before != book.top()) {
      topOfBookBuffers[stockLocate].push({std::chrono::high_resolution_clock::now(), book.top()});
    }
  };

  auto executeOrder = [&books, &topOfBookBuffers](auto stockLocate, auto oid, auto qty) {
    auto& book = books[stockLocate];
    auto before = book.top();
    switch (books[stockLocate].executeOrder(toOrderId(oid), toInt(qty))) {
      case lob::ExecuteOrderResult::FULL:
        std::println("Executed order {} (full: {})", oid, (int)qty);
        break;
      case lob::ExecuteOrderResult::PARTIAL:
        std::println("Executed order {} (partial: {})", oid, (int)qty);
        break;
      case lob::ExecuteOrderResult::ERROR:
        throw std::runtime_error(std::format("Could not execute order {} (qty: {})", oid, (int)qty));
    }
    if (before != book.top()) {
      topOfBookBuffers[stockLocate].push({std::chrono::high_resolution_clock::now(), book.top()});
    }
  };

  auto getNextMarketDataEvent = [&reader, &addOrder, &deleteOrder, &reduceOrder, &replaceOrder, &executeOrder] {
    return ::getNextMarketDataEvent(reader, addOrder, deleteOrder, reduceOrder, replaceOrder, executeOrder);
  };

  auto simulator = simulator::Simulator{getNextMarketDataEvent};

  using namespace std::chrono_literals;

  if (singleThreaded) {
    auto strategy = strategies::TrivialStrategy<TopOfBookBuffer>();
    auto const& book = books[symbols.byName("QQQ")];
    size_t bufferReadIdx = 0;
    for (int i : std::views::iota(0, numIters)) {
      simulator.step();
      strategy.onUpdate(book.top());
    }
    std::println("Strategy and simulation done:");
    auto const& diagnostics = strategy.diagnostics();
    diagnostics.print();
    diagnostics.save("diagnostics/ST_QQQ.json");

  } else {
    std::atomic<bool> running = true;

    auto const simulatorLoop = [&]() {
      std::this_thread::sleep_for(1s);

      for (int i : std::views::iota(0, numIters)) {
        simulator.step();
      }
      std::println("Simulation done.");
      running = false;
    };

    auto const strategyLoop = [&running, &symbols = std::as_const(symbols), &topOfBookBuffers = std::as_const(topOfBookBuffers)](std::string const& symbolName) {
      auto strategy = strategies::TrivialStrategy<TopOfBookBuffer>();
      auto const& topOfBookBuffer = topOfBookBuffers.at(symbols.byName(symbolName));
      size_t bufferReadIdx = 0;
      return strategy.loop(running, topOfBookBuffer, bufferReadIdx);
    };

    auto pool = exec::static_thread_pool(5);
    auto const& sched = pool.get_scheduler();

    namespace ex = stdexec;

    auto work = ex::when_all(
        ex::schedule(sched) | ex::then(pin_to_core<0>) | ex::then([&] { return strategyLoop("QQQ"); }) | ex::then([&](auto const& d) { d.save("diagnostics/MT_QQQ.json"); return d; }),
        ex::schedule(sched) | ex::then(pin_to_core<1>) | ex::then([&] { return strategyLoop("SPY"); }) | ex::then([&](auto const& d) { d.save("diagnostics/MT_SPY.json"); return d; }),
        ex::schedule(sched) | ex::then(pin_to_core<2>) | ex::then([&] { return strategyLoop("AMD"); }) | ex::then([&](auto const& d) { d.save("diagnostics/MT_AMD.json"); return d; }),
        ex::schedule(sched) | ex::then(pin_to_core<3>) | ex::then([&] { return strategyLoop("IWM"); }) | ex::then([&](auto const& d) { d.save("diagnostics/MT_IWM.json"); return d; }),
        ex::schedule(sched) | ex::then(pin_to_core<4>) | ex::then(simulatorLoop));

    auto diagnostics = ex::sync_wait(std::move(work)).value();

    std::println("Done!");

    auto f = [](auto const& diagnostics, size_t i) {
      std::println("Diagnostics {}:", i);
      diagnostics.print();
      std::println("");
    };

    tuple_map(diagnostics, f);
  }

} catch (std::exception const& ex) {
  std::println("Exception: {}", ex.what());
} catch (...) {
  std::println("Unknown exception");
}
