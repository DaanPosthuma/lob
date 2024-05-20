#include "ItchReader.h"

#include <md/BinaryDataReader.h>
#include <md/itch/MessageReaders.h>

#include <ranges>

using namespace std::string_literals;

int pymd::ItchReader::read(int num) const {

  auto reader = md::BinaryDataReader(mFile.data(), mFile.size());

  for (auto i : std::views::iota(0, num)) {
    if (reader.remaining() < 3) return i;
    auto const messageType = md::itch::currentMessageType(reader);
    auto const messageTypeAsChar = std::to_underlying(messageType);
    if (mCallbacks[messageTypeAsChar]) {
      DictType dict;
      dict["test"] = "test"s;
      mCallbacks[messageTypeAsChar](dict);
    } else {
    }

    md::itch::skipCurrentMessage(reader);
  }

  return num;
}
