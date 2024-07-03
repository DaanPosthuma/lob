#include "ItchBooksManager.h"

#include <md/itch/TypeFormatters.h>

#include <chrono>
#include <format>

#include "ItchToLobType.h"

void simulator::ItchBooksManager::addOrder(md::itch::types::locate_t stockLocate, md::itch::types::oid_t oid, md::itch::types::BUY_SELL buy, md::itch::types::qty_t qty, md::itch::types::price_t price) {
  auto& book = mBooks[stockLocate];
  auto before = book.top();
  book.addOrder(toOrderId(oid), toDirection(buy), toInt(qty), toLevel<LobT::Precision>(price));
  // std::println("Added order {}. Size: {}", oid, (int)qty);
  if (before != book.top()) {
    mTopOfBookBuffers[stockLocate].push({std::chrono::high_resolution_clock::now(), book.top()});
  }
}

void simulator::ItchBooksManager::deleteOrder(md::itch::types::locate_t stockLocate, md::itch::types::oid_t oid) {
  auto& book = mBooks[stockLocate];
  auto before = book.top();
  if (book.deleteOrder(toOrderId(oid))) {
    // std::println("Deleted order {}", oid);
  } else {
    throw std::runtime_error(std::format("Could not delete order {}", oid));
  }
  if (before != book.top()) {
    mTopOfBookBuffers[stockLocate].push({std::chrono::high_resolution_clock::now(), book.top()});
  }
}

void simulator::ItchBooksManager::replaceOrder(md::itch::types::locate_t stockLocate, md::itch::types::oid_t oid, md::itch::types::oid_t newOid, md::itch::types::qty_t newQty, md::itch::types::price_t newPrice) {
  auto& book = mBooks[stockLocate];
  auto before = book.top();
  if (book.replaceOrder(toOrderId(oid), toOrderId(newOid), toInt(newQty), toLevel<LobT::Precision>(newPrice))) {
    // std::println("Replaced order {} with {}", oid, newOid);
  } else {
    throw std::runtime_error(std::format("Could not replace order {} with {}", oid, newOid));
  }
  if (before != book.top()) {
    mTopOfBookBuffers[stockLocate].push({std::chrono::high_resolution_clock::now(), book.top()});
  }
}

void simulator::ItchBooksManager::reduceOrder(md::itch::types::locate_t stockLocate, md::itch::types::oid_t oid, md::itch::types::qty_t qty) {
  auto& book = mBooks[stockLocate];
  auto before = book.top();
  if (book.reduceOrder(toOrderId(oid), toInt(qty))) {
    // std::println("Reduced order {} by {}", oid, (int)qty);
  } else {
    throw std::runtime_error(std::format("Could not reduce order {} in book!!", oid, stockLocate));
  }
  if (before != book.top()) {
    mTopOfBookBuffers[stockLocate].push({std::chrono::high_resolution_clock::now(), book.top()});
  }
}

void simulator::ItchBooksManager::executeOrder(md::itch::types::locate_t stockLocate, md::itch::types::oid_t oid, md::itch::types::qty_t qty) {
  auto& book = mBooks[stockLocate];
  auto before = book.top();
  switch (book.executeOrder(toOrderId(oid), toInt(qty))) {
    case lob::ExecuteOrderResult::FULL:
      // std::println("Executed order {} (full: {})", oid, (int)qty);
      break;
    case lob::ExecuteOrderResult::PARTIAL:
      // std::println("Executed order {} (partial: {})", oid, (int)qty);
      break;
    case lob::ExecuteOrderResult::ERROR:
      throw std::runtime_error(std::format("Could not execute order {} (qty: {})", oid, (int)qty));
  }
  if (before != book.top()) {
    mTopOfBookBuffers[stockLocate].push({std::chrono::high_resolution_clock::now(), book.top()});
  }
}
