#include <gtest/gtest.h>

#include <md/MappedFile.h>
#include <md/BinaryDataReader.h>
#include <md/itch/MessageReaders.h>

using namespace std::string_literals;

namespace {

  TEST(ItchReader, Read) {
#ifdef _WIN32
    auto const filename = "C:\\dev\\VS\\lob\\data\\01302019.NASDAQ_ITCH50"s;
#else
    auto const filename = "/mnt/itch-data/01302019.NASDAQ_ITCH50";
#endif
    auto const file = md::MappedFile(filename);
    auto reader = md::BinaryDataReader(file.data(), file.size());
    size_t count = 0;
    /*while (reader.remaining() > 3) {
      auto const messageType = md::itch::currentMessageType(reader);
      md::itch::skip(messageType, reader);
      ++count;
    }*/
    std::cout << "Count: " << count << std::endl;

  }

}

