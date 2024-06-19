#pragma once

#include <algorithm>
#include <boost/unordered_map.hpp>
#include <cassert>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <print>
#include <ranges>

namespace lob {

template <class... Args>
using MapT = std::map<Args...>;
// using MapT = boost::container::flat_map<Args...>;

template <class... Args>
using UnorderedMapT = boost::unordered_map<Args...>;
// using UnorderedMapT = std::unordered_map<Args...>;

enum class ExecuteOrderResult {
  PARTIAL,
  FULL,
  ERROR
};

std::ostream& operator<<(std::ostream& out, ExecuteOrderResult result) {
  switch (result) {
    case ExecuteOrderResult::PARTIAL:
      out << "ExecuteOrderResult::PARTIAL";
      break;
    case ExecuteOrderResult::FULL:
      out << "ExecuteOrderResult::FULL";
      break;
    case ExecuteOrderResult::ERROR:
      out << "ExecuteOrderResult::ERROR";
      break;
  }
  return out;
}

class OrderId {
 public:
  explicit constexpr OrderId(int id) : mId(id) {}
  inline friend std::ostream& operator<<(std::ostream& ostr, OrderId const& order) {
    ostr << order.mId;
    return ostr;
  }

  inline friend auto constexpr operator<=>(OrderId const& lhs, OrderId const& rhs) noexcept = default;

  static OrderId Generate() {
    static int id = 9000000;
    return OrderId(id++);
  }

  explicit operator int() const {
    return mId;
  }

  friend struct std::hash<lob::OrderId>;
  friend struct boost::hash<lob::OrderId>;

 private:
  int mId;
};

consteval int ipow(int num, int pow) {
  return pow == 0 ? 1 : num * ipow(num, pow - 1);
}

template <int Precision>
struct PrecisionMultiplier {
  static constexpr double value = 1.0 / ipow(10, Precision);
};

template <int Precision>
class Level {
 public:
  explicit constexpr Level(int level) : mLevel(level) {}
  explicit constexpr operator int() const noexcept {
    return mLevel;
  }
  explicit constexpr operator double() const noexcept {
    return mLevel * PrecisionMultiplier<Precision>::value;
  }

  inline friend std::ostream& operator<<(std::ostream& ostr, Level const& level) noexcept {
    ostr << static_cast<double>(level);
    return ostr;
  }

  inline friend auto constexpr operator<=>(Level<Precision> lhs, Level<Precision> rhs) noexcept = default;

 private:
  int mLevel;
};

}  // namespace lob

namespace std {
template <>
struct hash<lob::OrderId> {
  size_t operator()(lob::OrderId const& orderId) const {
    return std::hash<size_t>{}(orderId.mId);
  }
};
}  // namespace std

namespace boost {
template <>
struct hash<lob::OrderId> {
  size_t operator()(lob::OrderId const& orderId) const {
    return boost::hash<size_t>{}(orderId.mId);
  }
};
}  // namespace boost

namespace lob {

enum class Direction {
  Buy,
  Sell
};

template <int Precision>
class LimitOrder {
 public:
  using LevelT = Level<Precision>;
  constexpr LimitOrder(int size, Direction direction, LevelT level, OrderId orderId) : mSize(size), mDirection(direction), mLevel(level), mOrderId(orderId) {}

  [[nodiscart]] constexpr auto size() const noexcept { return mSize; }
  [[nodiscart]] constexpr auto direction() const noexcept { return mDirection; }
  [[nodiscart]] constexpr auto level() const noexcept { return mLevel; }
  [[nodiscart]] constexpr auto orderId() const noexcept { return mOrderId; }
  constexpr auto setSize(int size) noexcept { mSize = size; }

 private:
  int mSize;
  Direction mDirection;
  LevelT mLevel;
  OrderId mOrderId;
};

template <int Precision>
class LevelOrders {
 public:
  auto add(int size, Direction direction, Level<Precision> level, OrderId orderId) {
    mDepth += size;
    return mOrders.emplace(mOrders.begin(), size, direction, level, orderId);
  }

  void remove(std::list<LimitOrder<Precision>>::iterator it) {
    mDepth -= it->size();
    mOrders.erase(it);
  }

  auto timePriority(std::list<LimitOrder<Precision>>::const_iterator it) const {
    return std::pair(mOrders.size(), std::distance(it, mOrders.end()));
  }

  void print() const {
    std::println("depth: {}. num orders: {}", mDepth, mOrders.size());
    for (auto const& order : mOrders) {
      std::println("[{}, {}]", (int)order.orderId(), order.size());
    }
  }

  void reduce(int oldSize, int newSize) {
    mDepth += newSize - oldSize;
  }

  [[nodiscard]] auto empty() const noexcept { return mOrders.empty(); }

  auto& oldest() {
    return *mOrders.rbegin();
  }

  void deleteOldest() {
    mDepth -= mOrders.back().size();
    mOrders.pop_back();
  }

  [[nodiscard]] int depth() const noexcept { return mDepth; }
  [[nodiscard]] auto num() const noexcept { return mOrders.size(); }

 private:
  std::list<LimitOrder<Precision>> mOrders;
  int mDepth = 0;
};

template <int Precision>
class LimitOrderBook {
 public:
  static constexpr int Precision = Precision;
  using LevelT = Level<Precision>;

  LimitOrderBook()
      : mBid([](auto lhs, auto rhs) { return lhs < rhs; }),
        mAsk([](auto lhs, auto rhs) { return lhs > rhs; }) {}

  OrderId addOrder(const Direction direction, const int size, const LevelT level) {
    return addOrder(OrderId::Generate(), direction, size, level);
  }

  OrderId addOrder(const OrderId orderId, const Direction direction, const int size, const LevelT level) {
    // todo(?): check if we can (partially) trade
    auto it = (direction == Direction::Sell ? mAsk : mBid)[level].add(size, direction, level, orderId);
    mOrders.emplace(orderId, it);

    return orderId;
  }

  bool deleteOrder(const OrderId orderId) {
    auto const it = mOrders.find(orderId);
    if (it == mOrders.end()) return false;

    auto const orderIt = it->second;
    auto const level = orderIt->level();
    auto& side = orderIt->direction() == Direction::Sell ? mAsk : mBid;
    side[level].remove(orderIt);
    if (side[level].empty()) side.erase(level);
    mOrders.erase(it);

    return true;
  }

  bool reduceOrder(const OrderId orderId, int numCancelled) {
    auto const it = mOrders.find(orderId);
    if (it == mOrders.end()) return false;

    auto const orderIt = it->second;

    auto const newSize = orderIt->size() - numCancelled;
    if (newSize <= 0) {
      throw std::runtime_error(std::format("reduceOrder: Reduced level to {}!", newSize));
    }

    auto const level = orderIt->level();
    auto& side = orderIt->direction() == Direction::Sell ? mAsk : mBid;
    side[level].reduce(orderIt->size(), newSize);
    orderIt->setSize(newSize);

    return true;
  }

  bool replaceOrder(const OrderId orderId, const OrderId newOrderId, int newSize, const LevelT newLevel) {
    if (newSize == 0) {
      throw std::runtime_error("replaceOrder: new size is 0!");
    }
    auto const it = mOrders.find(orderId);
    if (it == mOrders.end()) return false;

    auto const orderIt = it->second;
    auto const oldLevel = orderIt->level();
    auto const direction = orderIt->direction();

    auto& side = direction == Direction::Sell ? mAsk : mBid;

    if (newLevel == oldLevel) {
      // std::println("replaceOrder: new level same as the old one! client probably should have reduced (partially cancelled) order to retain time priority. Or is this case handled as a reduce by the exchange?");
    }

    side[oldLevel].remove(orderIt);
    if (side[oldLevel].empty()) side.erase(oldLevel);
    mOrders.erase(it);

    addOrder(newOrderId, direction, newSize, newLevel);

    return true;
  }

  ExecuteOrderResult executeOrder(const OrderId orderId, int size) {
    auto const it = mOrders.find(orderId);
    if (it == mOrders.end()) return ExecuteOrderResult::ERROR;

    auto const orderIt = it->second;
    auto const level = orderIt->level();
    auto& side = orderIt->direction() == Direction::Sell ? mAsk : mBid;

    auto const bs = orderIt->direction() == Direction::Sell ? 'S' : 'B';

    if (level != side.rbegin()->first) {
      std::println("Executing {}, but it's not the best price (bs: {})", (int)orderId, bs);
      std::println("best level: {}. ", static_cast<int>(side.rbegin()->first), static_cast<double>(side.rbegin()->first));
      std::print("level: {} ({}). ", static_cast<int>(level), static_cast<double>(level));
      std::cout << *this << std::endl;
    }

    
    if (auto [total, priority] = side[level].timePriority(orderIt); priority != 1) {
      std::println("Executing {}, but not best time priority (priority: {}, total: {}, bs: {})", (int)orderId, priority, total, bs);
      std::print("level: {} ({}). ", static_cast<int>(level), static_cast<double>(level));
      side[level].print();
    }

    if (orderIt->size() == size) {
      side[level].remove(orderIt);
      if (side[level].empty()) side.erase(level);
      mOrders.erase(it);
      return ExecuteOrderResult::FULL;
    } else {
      auto newSize = orderIt->size() - size;
      if (newSize < 0) {
        throw std::runtime_error(std::format("executeOrder: Reduced level to {}!", newSize));
      }
      side[level].reduce(orderIt->size(), newSize);
      orderIt->setSize(newSize);
      return ExecuteOrderResult::PARTIAL;
    }
  }

  [[nodiscard]] auto hasBids() const noexcept {
    return !mBid.empty();
  }

  [[nodiscard]] auto hasAsks() const noexcept {
    return !mAsk.empty();
  }

  [[nodiscard]] auto bid() const noexcept {
    return mBid.rbegin()->first;
  }

  [[nodiscard]] auto ask() const noexcept {
    return mAsk.rbegin()->first;
  }

  [[nodiscard]] int bidDepth() const noexcept {
    return mBid.rbegin()->second.depth();
  }

  [[nodiscard]] int askDepth() const noexcept {
    return mAsk.rbegin()->second.depth();
  }

  struct TopOfBook {
    LevelT bid{0};
    int bidDepth = 0;
    LevelT ask{0};
    int askDepth = 0;

    inline friend auto constexpr operator<=>(TopOfBook lhs, TopOfBook rhs) noexcept = default;
  };

  [[nodiscard]] auto top() const noexcept {
    TopOfBook top{};
    if (hasBids()) {
      top.bid = bid();
      top.bidDepth = bidDepth();
    }
    if (hasAsks()) {
      top.ask = ask();
      top.askDepth = askDepth();
    }
    return top;
  }

 private:
  UnorderedMapT<OrderId, typename std::list<LimitOrder<Precision>>::iterator> mOrders;
  MapT<LevelT, LevelOrders<Precision>, std::function<bool(LevelT, LevelT)>> mBid;
  MapT<LevelT, LevelOrders<Precision>, std::function<bool(LevelT, LevelT)>> mAsk;

  inline friend std::ostream& operator<<(std::ostream& ostr, LimitOrderBook const& book) noexcept {
    ostr << "[ LimitOrderBook begin ]" << std::endl;
    ostr << "Orders: ";
    std::ranges::for_each(book.mOrders | std::views::keys, [&ostr, first = true](auto orderId) mutable { ostr << (first ? "" : ",") << orderId; first = false; });

    ostr << std::endl;
    ostr << "Bids: " << std::endl;
    for (auto const& [level, orders] : book.mBid) {
      ostr << "Level " << level << ", num: " << orders.num() << ", total depth " << orders.depth() << std::endl;
    }
    ostr << "Asks: " << std::endl;
    for (auto const& [level, orders] : book.mAsk) {
      ostr << "Level " << level << ", num: " << orders.num() << ", total depth " << orders.depth() << std::endl;
    }
    ostr << "[ LimitOrderBook end ]" << std::endl;
    return ostr;
  }
};

}  // namespace lob
