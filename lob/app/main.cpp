#include <lob/lob.h>
#include <md/BinaryDataReader.h>
#include <md/MappedFile.h>
#include <md/itch/MessageReaders.h>

#include <iostream>

namespace {

using namespace std::string_literals;

auto getTestFile() {
#ifdef _WIN32
  auto const filename = "C:\\dev\\VS\\lob\\data\\01302019.NASDAQ_ITCH50"s;
#else
  auto const filename = "/mnt/itch-data/01302019.NASDAQ_ITCH50";
#endif
  return md::MappedFile(filename);
}

auto constexpr static maxCount = 100000000;

}  // namespace

int main() {
  std::cout << "Start" << std::endl;

  auto const file = getTestFile();
  auto reader = md::BinaryDataReader(file.data(), file.size());
  auto count = size_t(0);

  while (count != maxCount && reader.remaining() > 3) {
    auto const currentMessageType = md::itch::currentMessageType(reader);
    std::cout << (char)currentMessageType;
    md::itch::skipCurrentMessage(reader);
    /*switch (currentMessageType) {
      case md::itch::messages::MessageType::STOCK_DIRECTORY: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::STOCK_DIRECTORY>(reader);
        stockMap[msg.stock_locate] = msg.stock;
        break;
      }
      case md::itch::messages::MessageType::ADD_ORDER: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::ADD_ORDER>(reader);
        ++securityCount[msg.stock_locate];
        break;
      }
      default:
        md::itch::skipCurrentMessage(reader);
    }*/

    ++count;
  }
}
