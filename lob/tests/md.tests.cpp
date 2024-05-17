#include <gtest/gtest.h>

#include <md/MappedFile.h>

using namespace std::string_literals;

namespace {

  TEST(ItchReader, Read) {
#ifdef _WIN32
    auto const filename = "C:\\dev\\VS\\lob\\data\\01302019.NASDAQ_ITCH50"s;
#else
    auto const filename = "/mnt/itch-data/01302019.NASDAQ_ITCH50";
#endif
    auto file = md::MappedFile(filename);
    

  }

}

