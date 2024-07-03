#pragma once

#include <md/BinaryDataReader.h>
#include <md/itch/messages.h>
#include <md/itch/types.h>

#include <array>
#include <cassert>
#include <functional>

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

inline auto currentMessageType(md::BinaryDataReader const& reader) {
  return md::itch::types::MessageType(*reader.get(0 + 2));
}

inline auto currentMessageTimestamp(md::BinaryDataReader const& reader) {
  return md::itch::messages::read_timestamp(reader.get(5 + 2));
}

inline void skipCurrentMessage(md::BinaryDataReader& reader) {
  auto const msglen = be16toh(*(uint16_t*)reader.get(0));
  reader.advance(msglen + 2);
}

}  // namespace md::itch
