#pragma once

#include "BinaryDataReader.h"
#include "md/itch/MessageReaders.h"
#include <limits>
#include <string>
#include <vector>
#include <boost/algorithm/string/trim.hpp>

namespace md::utils {

class Symbols {
 public:
  explicit Symbols(md::BinaryDataReader& reader) {
    bool preTrading = true;

    while (preTrading) {
      auto const currentMessageType = md::itch::currentMessageType(reader);
      switch (currentMessageType) {
        case md::itch::messages::MessageType::SYSEVENT: {
          auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::SYSEVENT>(reader);
          if (msg.eventCode == 'S') preTrading = false;
          break;
        }
        case md::itch::messages::MessageType::STOCK_DIRECTORY: {
          auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::STOCK_DIRECTORY>(reader);
          auto const name = boost::algorithm::trim_right_copy(msg.stock);
          mIdToName[msg.stock_locate] = name;
          mNameToId[name] = msg.stock_locate;
          break;
        }
        default:
          md::itch::skipCurrentMessage(reader);
      }
    }
  }

  [[nodiscard]] auto count() const noexcept {
    return mNameToId.size();
  }

  [[nodiscard]] auto byName(std::string const& name) const {
    if (!mNameToId.contains(name)) throw std::runtime_error("Symbol not found");
    return mNameToId.at(name);
  }

  [[nodiscard]] auto byId(uint16_t id) const {
    if (mIdToName[id].empty()) throw std::runtime_error("Id not found");
    return mIdToName[id];
  }

  [[nodiscard]] auto begin() const {
    return mNameToId.begin();
  }

  [[nodiscard]] auto end() const {
    return mNameToId.end();
  }

 private:
  std::vector<std::string> mIdToName = {std::numeric_limits<uint16_t>::max(), {}, {}};
  std::unordered_map<std::string, int> mNameToId = {};
};

}  // namespace md::utils
