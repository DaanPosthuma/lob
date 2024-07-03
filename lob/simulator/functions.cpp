#include "functions.h"

#include <md/BinaryDataReader.h>
#include <md/Symbols.h>
#include <md/itch/MessageReaders.h>
#include <strategies/Strategies.h>

#include <chrono>
#include <exec/inline_scheduler.hpp>
#include <exec/static_thread_pool.hpp>
#include <print>
#include <stdexec/execution.hpp>
#include <utility>

#include "ItchBooksManager.h"
#include "ItchToLobType.h"
#include "PinToCore.h"
#include "Simulator.h"
#include "TupleMap.h"

simulator::Simulator::EventT simulator::getNextMarketDataEvent(md::BinaryDataReader& reader, auto& bmgr) {
  while (reader.remaining() >= 3) {
    auto const currentMessageType = md::itch::currentMessageType(reader);
    auto const start = std::chrono::high_resolution_clock::now();
    switch (currentMessageType) {
      case md::itch::messages::MessageType::ADD_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::ADD_ORDER>(reader);
        return {msg.timestamp, [msg, &bmgr] {
                  bmgr.addOrder(msg.stock_locate, msg.oid, msg.buy, msg.qty, msg.price);
                }};
      }
      case md::itch::messages::MessageType::ADD_ORDER_MPID: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::ADD_ORDER_MPID>(reader);
        return {msg.add_msg.timestamp, [msg = msg.add_msg, &bmgr] {
                  bmgr.addOrder(msg.stock_locate, msg.oid, msg.buy, msg.qty, msg.price);
                }};
      }
      case md::itch::messages::MessageType::REPLACE_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::REPLACE_ORDER>(reader);
        return {msg.timestamp, [msg, &bmgr] {
                  bmgr.replaceOrder(msg.stock_locate, msg.oid, msg.new_order_id, msg.new_qty, msg.new_price);
                }};
      }
      case md::itch::messages::MessageType::REDUCE_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::REDUCE_ORDER>(reader);
        return {msg.timestamp, [msg, &bmgr] {
                  bmgr.reduceOrder(msg.stock_locate, msg.oid, msg.qty);
                }};
        break;
      }
      case md::itch::messages::MessageType::EXECUTE_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::EXECUTE_ORDER>(reader);
        return {msg.timestamp, [msg, &bmgr] {
                  bmgr.executeOrder(msg.stock_locate, msg.oid, msg.qty);
                }};
      }
      case md::itch::messages::MessageType::EXECUTE_ORDER_WITH_PRICE: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::EXECUTE_ORDER_WITH_PRICE>(reader);
        return {msg.exec.timestamp, [msg = msg.exec, &bmgr] {
                  bmgr.executeOrder(msg.stock_locate, msg.oid, msg.qty);
                }};
      }
      case md::itch::messages::MessageType::DELETE_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::DELETE_ORDER>(reader);
        return {msg.timestamp, [msg, &bmgr] {
                  bmgr.deleteOrder(msg.stock_locate, msg.oid);
                }};
      }
      default:
        md::itch::skipCurrentMessage(reader);
    }
  }
  throw std::runtime_error("end of messages");
}

void simulator::runTest(md::BinaryDataReader& reader, int numIters, bool singleThreaded) try {
  using LobT = lob::LimitOrderBook;
  using TopOfBookBuffer = RingBuffer<std::pair<std::chrono::high_resolution_clock::time_point, LobT::TopOfBook>, 64>;

  std::println("Loading test file...");

  auto const symbols = md::utils::Symbols(reader);

  std::println("Loaded {} symbols", symbols.count());

  ItchBooksManager bmgr;

  auto simulator = simulator::Simulator{[&] { return getNextMarketDataEvent(reader, bmgr); }};

  using namespace std::chrono_literals;

  if (singleThreaded) {
    auto strategy = strategies::TrivialStrategy();
    auto const& book = bmgr.bookById(symbols.byName("QQQ"));
    auto prevTop = book.top();
    size_t bufferReadIdx = 0;
    for (int i : std::views::iota(0, numIters)) {
      auto timestamp = simulator.step();
      auto const top = book.top();
      if (prevTop != top) {
        strategy.onUpdate(timestamp, book.top());
        prevTop = top;
      }
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

    auto const strategyLoop = [&running, &symbols = std::as_const(symbols), &bmgr](std::string const& symbolName) {
      auto strategy = strategies::TrivialStrategy();
      auto const& topOfBookBuffer = bmgr.bufferById(symbols.byName(symbolName));
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
