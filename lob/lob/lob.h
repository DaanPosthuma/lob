#pragma once

#include <algorithm>
#include <boost/container/flat_map.hpp>
#include <boost/unordered_map.hpp>
#include <cassert>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <ranges>
#include <set>
#include <shared_mutex>
#include <string_view>
#include <vector>

template <class... Args>
using MapT = std::map<Args...>;
// using MapT = boost::container::flat_map<Args...>;

template <class... Args>
using UnorderedMapT = boost::unordered_map<Args...>;
// using UnorderedMapT = std::unordered_map<Args...>;

namespace lob {

class OrderId {
 public:
  explicit OrderId(int id) : mId(id) {}
  inline friend std::ostream& operator<<(std::ostream& ostr, OrderId const& order) {
    ostr << order.mId;
    return ostr;
  }

  inline friend auto operator==(OrderId const& lhs, OrderId const& rhs) {
    return lhs.mId == rhs.mId;
  }

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

template <int Precision>
struct PrecisionMultiplier {};

template <>
struct PrecisionMultiplier<4> {
  static constexpr double value = 1 / 10000.;
};

template <int Precision>
class Level {
 public:
  Level(int level) : mLevel(level) {}
  operator int() const {
    return mLevel;
  }
  operator double() const {
    return mLevel * PrecisionMultiplier<Precision>::value;
  }

  inline friend std::ostream& operator<<(std::ostream& ostr, Level const& level) {
    ostr << static_cast<double>(level);
    return ostr;
  }

  inline friend auto operator<=>(Level<Precision> const& lhs, Level<Precision> const& rhs) = default;

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
  LimitOrder(int size, Direction direction, Level<Precision> level, OrderId orderId) : mSize(size), mDirection(direction), mLevel(level), mOrderId(orderId) {}

  auto size() const { return mSize; }
  auto direction() const { return mDirection; }
  auto level() const { return mLevel; }
  auto orderId() const { return mOrderId; }

  /*LimitOrder createSizeAmendedOrder(int newSize) const {
    return LimitOrder(newSize, mLevel, mOrderId);
  }*/

 private:
  int mSize;
  Direction mDirection;     // todo: remove
  Level<Precision> mLevel;  // todo: remove
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
  auto empty() const { return mOrders.empty(); }
  auto& oldest() {
    return *mOrders.rbegin();
  }
  void deleteOldest() {
    mDepth -= mOrders.back().size();
    mOrders.pop_back();
  }

  int depth() const { return mDepth; }
  auto num() const { return mOrders.size(); }

 private:
  std::list<LimitOrder<Precision>> mOrders;
  int mDepth = 0;
};

template <int Precision>
class LimitOrderBook {
 public:
  LimitOrderBook() : mBid([](int lhs, int rhs) { return lhs < rhs; }), mAsk([](int lhs, int rhs) { return lhs > rhs; }) {}

  OrderId addOrder(const Direction direction, const int size, const Level<Precision> level) {
    return addOrder(OrderId::Generate(), direction, size, level);
  }

  OrderId addOrder(const OrderId orderId, const Direction direction, const int size, const Level<Precision> level) {
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
    Level<Precision> bid = 0;
    int bidDepth = 0;
    Level<Precision> ask = 0;
    int askDepth = 0;

    inline friend auto operator<=>(TopOfBook lhs, TopOfBook rhs) = default;
  };

  [[nodiscard]] auto top() const noexcept {
    TopOfBook ret = {};
    if (hasBids()) {
      ret.bid = bid();
      ret.bidDepth = bidDepth();
    }
    if (hasAsks()) {
      ret.ask = ask();
      ret.askDepth = askDepth();
    }
    return ret;
  }

  /*void PlaceMarketOrder(const Direction direction, const int size)
  {
    //std::print("MO (direction={}, size={})", direction, size);

    auto& orders = direction == Direction::Sell ? mBid : mAsk;

    int outstanding = size;
    double averagePrice = 0;

    while (outstanding > 0)
    {
      auto max = orders.rbegin()->first;
      while (orders[max].empty())
      {
        orders.erase(max);
        max = orders.rbegin()->first;
      }
      auto& ordersForLevel = orders.rbegin()->second;

      const auto& order = ordersForLevel.oldest();

      if (order.GetSize() <= outstanding)
      {
        //std::cout << "(Partial) fill" << std::endl;
        outstanding -= order.GetSize();
        averagePrice += order.GetSize() * max * PrecisionMultiplier<Precision>::value / size;
        //order.print();

        ordersForLevel.deleteOldest();
      }
      else if (order.GetSize() > outstanding)
      {
        //std::cout << "Next LO is larger than MO" << std::endl;
        //order.print();
        const auto amended = order.createSizeAmendedOrder(order.GetSize() - outstanding);
        //std::cout << "Amended:" << std::endl;
        //amended.print();
        ordersForLevel.oldest() = amended;
        averagePrice += outstanding * max * PrecisionMultiplier<Precision>::value / size;
        outstanding = 0;

      }
    }
    std::cout << "Done, effective price: " << averagePrice << std::endl;
  }*/

 private:
  UnorderedMapT<OrderId, typename std::list<LimitOrder<Precision>>::iterator> mOrders;
  MapT<Level<Precision>, LevelOrders<Precision>, std::function<bool(Level<Precision>, Level<Precision>)>> mBid;
  MapT<Level<Precision>, LevelOrders<Precision>, std::function<bool(Level<Precision>, Level<Precision>)>> mAsk;

  inline friend std::ostream& operator<<(std::ostream& ostr, LimitOrderBook const& book) {
    ostr << "[ LimitOrderBook begin ]" << std::endl;
    ostr << "Orders: ";
    std::ranges::for_each(book.mOrders | std::views::keys, [first = true](auto orderId) mutable { std::cout << (first ? "" : ",") << orderId; first = false; });

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

template <int Precision>
class LimitOrderBookWithLocks : private LimitOrderBook<Precision> {
 public:
  OrderId addOrder(const OrderId orderId, const Direction direction, const int size, const Level<Precision> level) {
    std::unique_lock const lock(mMutex);
    return LimitOrderBook<Precision>::addOrder(orderId, direction, size, level);
  }

  bool deleteOrder(const OrderId orderId) {
    std::unique_lock const lock(mMutex);
    return LimitOrderBook<Precision>::deleteOrder(orderId);
  }

  [[nodiscard]] auto hasBids() const noexcept {
    std::shared_lock const lock(mMutex);
    return LimitOrderBook<Precision>::hasBids();
  }

  [[nodiscard]] auto hasAsks() const noexcept {
    std::shared_lock const lock(mMutex);
    return LimitOrderBook<Precision>::hasAsks();
  }

  [[nodiscard]] auto bid() const noexcept {
    std::shared_lock const lock(mMutex);
    return LimitOrderBook<Precision>::bid();
  }

  [[nodiscard]] auto ask() const noexcept {
    std::shared_lock const lock(mMutex);
    return LimitOrderBook<Precision>::ask();
  }

  [[nodiscard]] int bidDepth() const noexcept {
    std::shared_lock const lock(mMutex);
    return LimitOrderBook<Precision>::bidDepth();
  }

  [[nodiscard]] int askDepth() const noexcept {
    std::shared_lock const lock(mMutex);
    return LimitOrderBook<Precision>::askDepth();
  }

  [[nodiscard]] auto top() const noexcept {
    std::shared_lock const lock(mMutex);
    return LimitOrderBook<Precision>::top();
  }

  inline friend std::ostream& operator<<(std::ostream& ostr, LimitOrderBookWithLocks const& book) {
    return ostr << static_cast<LimitOrderBook<Precision>>(book);
  }

 private:
  mutable std::shared_mutex mMutex;
};

}  // namespace lob
