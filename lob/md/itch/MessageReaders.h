#pragma once

#include <md/BinaryDataReader.h>
#include <md/itch/messages.h>
#include <md/itch/types.h>

namespace md::itch {

template <md::itch::types::MessageType messageType>
inline md::itch::messages::itch_message<messageType> readItchMessage(md::BinaryDataReader& buf) {
  auto const msglen = be16toh(*(uint16_t*)buf.get(0));
  buf.advance(2);
  assert(msglen == md::itch::messages::netlen<messageType>);

  auto const ret = md::itch::messages::itch_message<messageType>::parse(buf.get(0));
  buf.advance(md::itch::messages::netlen<messageType>);
  return ret;
}

template <md::itch::types::MessageType messageType>
inline void skipItchMessage(md::BinaryDataReader& buf) {
  auto const msglen = be16toh(*(uint16_t*)buf.get(0));
  buf.advance(2);
  assert(msglen == md::itch::messages::netlen<messageType>);

  buf.advance(md::itch::messages::netlen<messageType>);
}

#define ADDSKIPFUN(MT) functions[std::to_underlying(MT)] = skipItchMessage<MT>

inline void skip(md::itch::types::MessageType messageType, md::BinaryDataReader& reader) {
  auto static const functions = [] {
    auto functions = std::array<std::function<void(md::BinaryDataReader&)>, 256>();
    ADDSKIPFUN(types::MessageType::SYSEVENT);
    ADDSKIPFUN(types::MessageType::STOCK_DIRECTORY);
    ADDSKIPFUN(types::MessageType::TRADING_ACTION);
    ADDSKIPFUN(types::MessageType::REG_SHO_RESTRICT);
    ADDSKIPFUN(types::MessageType::MPID_POSITION);
    ADDSKIPFUN(types::MessageType::MWCB_DECLINE);
    ADDSKIPFUN(types::MessageType::MWCB_STATUS);
    ADDSKIPFUN(types::MessageType::IPO_QUOTE_UPDATE);
    ADDSKIPFUN(types::MessageType::ADD_ORDER);
    ADDSKIPFUN(types::MessageType::ADD_ORDER_MPID);
    ADDSKIPFUN(types::MessageType::EXECUTE_ORDER);
    ADDSKIPFUN(types::MessageType::EXECUTE_ORDER_WITH_PRICE);
    ADDSKIPFUN(types::MessageType::REDUCE_ORDER);
    ADDSKIPFUN(types::MessageType::DELETE_ORDER);
    ADDSKIPFUN(types::MessageType::REPLACE_ORDER);
    ADDSKIPFUN(types::MessageType::TRADE);
    ADDSKIPFUN(types::MessageType::CROSS_TRADE);
    ADDSKIPFUN(types::MessageType::BROKEN_TRADE);
    ADDSKIPFUN(types::MessageType::NET_ORDER_IMBALANCE);
    ADDSKIPFUN(types::MessageType::RETAIL_PRICE_IMPROVEMENT);
    ADDSKIPFUN(types::MessageType::PROCESS_LULD_AUCTION_COLLAR_MESSAGE);
    return functions;
  }();
  return functions[std::to_underlying(messageType)](reader); // better be sure every message type has a skip function
}

#undef ADDSKIPFUN

inline auto currentMessageType(md::BinaryDataReader const& reader) {
  return md::itch::types::MessageType(*reader.get(2));
}

}  // namespace md::itch
