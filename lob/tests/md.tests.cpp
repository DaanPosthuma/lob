#include <gtest/gtest.h>

#include <md/itch_reader.h>

namespace {

  TEST(ItchReader, Read) {
#ifdef _WIN32
    itch_reader::read("C:\\dev\\VS\\lob\\01302019.NASDAQ_ITCH50");
#else
    itch_reader::read("/mnt/itch-data/01302019.NASDAQ_ITCH50");
#endif
  }

}

