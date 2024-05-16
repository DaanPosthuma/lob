#pragma once

#include <iostream>
#include <vector>
#include <functional>
#include <ranges>
#include <algorithm>
#include <map>
#include <set>
#include <string_view>
#include <cassert>
#include <deque>

namespace lob {
  	
  class LimitOrder
  {
  public:
    LimitOrder(int size, int orderId)
      : mSize(size), mOrderId(orderId)
    {
      //std::cout << "Created LO (size=" << size << ", level=" << level << ", time=" << time << ", orderId=" << orderId << ")" << std::endl;
    }

    void print() const
    {
      std::cout << "LO (size=" << mSize << ", orderId=" << mOrderId << ")" << std::endl;
    }

    int GetSize() const
    {
      return mSize;
    }

    LimitOrder createSizeAmendedOrder(int newSize) const
    {
      return LimitOrder(newSize, mOrderId);
    }

  private:
    int mSize;
    int mTime;
    int mOrderId;
  };

  enum class Direction
  {
    Buy, Sell
  };
  
  class Level {
  public:
    void add(int size, int orderId) { mOrders.emplace_front(size, orderId); }
    auto empty() const { return mOrders.empty(); }
    auto& oldest() {
      return *mOrders.rbegin();
    }
    void deleteOldest() {
      mOrders.pop_back();
    }

  private:
    std::deque<LimitOrder> mOrders;
  };

  template <int Precision>
  struct PrecisionMultiplier {};

  template <>
  struct PrecisionMultiplier<4> {
    static constexpr double value = 1/10000.;
  };

  template <int Precision>
  class LimitOrderBook
  {
  public:
    LimitOrderBook()
      : mBid([](const double lhs, const double rhs) { return lhs < rhs; }), mAsk([](double lhs, double rhs) { return lhs > rhs; })
    {
    }

    int PlaceLimitOrder(const Direction direction, const int size, const int level)
    {
      // todo: check if we can (partially) trade
      auto const orderId = GenerateOrderId();
      (direction == Direction::Sell ? mAsk : mBid)[level].add(size, orderId);
      return orderId;
    }

    void PlaceMarketOrder(const Direction direction, const int size)
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
    }

  private:

    int GenerateOrderId() const
    {
      static int id = 900000;
      return id++;
    }

    std::map<int, Level, std::function<bool(double, double)>> mBid;
    std::map<int, Level, std::function<bool(double, double)>> mAsk;
  };

}

