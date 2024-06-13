#pragma once

#include <algorithm>
#include <boost/unordered_map.hpp>
#include <cassert>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <ranges>
#include <shared_mutex>

template <class... Args>
using MapT = std::map<Args...>;
// using MapT = boost::container::flat_map<Args...>;

template <class... Args>
using UnorderedMapT = boost::unordered_map<Args...>;
// using UnorderedMapT = std::unordered_map<Args...>;

namespace lob {

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

template <int Precision>
struct PrecisionMultiplier {};

template <>
struct PrecisionMultiplier<4> {
  static constexpr double value = 1 / 10000.;
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

  constexpr auto size() const noexcept { return mSize; }
  constexpr auto direction() const noexcept { return mDirection; }
  constexpr auto level() const noexcept { return mLevel; }
  constexpr auto orderId() const noexcept { return mOrderId; }

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

    updateTop(); // todo: only update when best bid or lowest ask is affected

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

    updateTop(); // todo: only update when best bid or lowest ask is changed

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
    LevelT bid{0};
    int bidDepth = 0;
    LevelT ask{0};
    int askDepth = 0;

    inline friend auto constexpr operator<=>(TopOfBook lhs, TopOfBook rhs) noexcept = default;
  };

  [[nodiscard]] auto top() const noexcept {
    return mTop;
  }

  [[nodiscard]] auto updateTop() noexcept {
    if (hasBids()) {
      mTop.bid = bid();
      mTop.bidDepth = bidDepth();
    }
    if (hasAsks()) {
      mTop.ask = ask();
      mTop.askDepth = askDepth();
    }
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
  MapT<LevelT, LevelOrders<Precision>, std::function<bool(LevelT, LevelT)>> mBid;
  MapT<LevelT, LevelOrders<Precision>, std::function<bool(LevelT, LevelT)>> mAsk;
  TopOfBook mTop = {};

  inline friend std::ostream& operator<<(std::ostream& ostr, LimitOrderBook const& book) noexcept {
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
  using LevelT = LimitOrderBook<Precision>::LevelT;
  OrderId addOrder(const OrderId orderId, const Direction direction, const int size, const LevelT level) {
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
