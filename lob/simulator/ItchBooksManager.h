#pragma once

#include <lob/RingBuffer.h>
#include <lob/lob.h>
#include <md/itch/types.h>

#include <unordered_map>

namespace simulator {

class ItchBooksManager {
 public:
  using LobT = lob::LimitOrderBook;
  using TopOfBookBuffer = RingBuffer<std::pair<std::chrono::high_resolution_clock::time_point, LobT::TopOfBook>, 64>;

  void addOrder(md::itch::types::locate_t stockLocate, md::itch::types::oid_t oid, md::itch::types::BUY_SELL buy, md::itch::types::qty_t qty, md::itch::types::price_t price);
  void deleteOrder(md::itch::types::locate_t stockLocate, md::itch::types::oid_t oid);
  void replaceOrder(md::itch::types::locate_t stockLocate, md::itch::types::oid_t oid, md::itch::types::oid_t newOid, md::itch::types::qty_t newQty, md::itch::types::price_t newPrice);
  void reduceOrder(md::itch::types::locate_t stockLocate, md::itch::types::oid_t oid, md::itch::types::qty_t qty);
  void executeOrder(md::itch::types::locate_t stockLocate, md::itch::types::oid_t oid, md::itch::types::qty_t qty);

  [[nodiscard]] auto& bookById(int id) {
    return mBooks[id];
  }

  [[nodiscard]] auto const& bookById(int id) const {
    return mBooks.at(id);
  }

  [[nodiscard]] auto& bufferById(int id) {
    return mTopOfBookBuffers[id];
  }

  [[nodiscard]] auto const& bufferById(int id) const {
    return mTopOfBookBuffers.at(id);
  }

 private:
  boost::unordered_map<int, LobT> mBooks;
  boost::unordered_map<int, TopOfBookBuffer> mTopOfBookBuffers;
};

}  // namespace simulator
