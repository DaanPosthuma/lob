#pragma once

#include <lob/lob.h>
#include <md/itch/types.h>

#include <utility>

namespace md::itch::types {

template <class ToT, class FromT>
inline void assertCanCastTo(FromT from) {
  if constexpr (!std::is_same<FromT, ToT>::value) {
    if (from > std::numeric_limits<ToT>::max())
      throw std::runtime_error(std::string("Cannot cast ") + std::to_string(from) + " from " + typeid(FromT).name() + " to " + typeid(ToT).name());
  }
}

inline auto toLobType(md::itch::types::oid_t oid) {
  assertCanCastTo<int>(std::to_underlying(oid));
  return lob::OrderId(static_cast<int>(oid));
}

inline auto toLobType(md::itch::types::BUY_SELL bs) {
  return bs == md::itch::types::BUY_SELL::BUY ? lob::Direction::Buy : lob::Direction::Sell;
}

inline auto toLobType(md::itch::types::price_t price) {
  assertCanCastTo<int>(std::to_underlying(price));
  return static_cast<int>(price);
}

inline auto toLobType(md::itch::types::qty_t qty) {
  assertCanCastTo<int>(std::to_underlying(qty));
  return static_cast<int>(qty);
}

}  // namespace md::itch::types