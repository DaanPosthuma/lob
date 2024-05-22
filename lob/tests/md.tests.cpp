#include <gtest/gtest.h>
#include <md/BinaryDataReader.h>
#include <md/MappedFile.h>
#include <md/itch/MessageReaders.h>

#include <ranges>

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

auto HHMM(md::itch::types::timestamp_t timestamp) {

    auto const hours = std::chrono::duration_cast<std::chrono::hours>(timestamp);
    auto const minutes = std::chrono::duration_cast<std::chrono::minutes>(timestamp - hours);

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours.count() << ":"
        << std::setw(2) << std::setfill('0') << minutes.count();

    return oss.str();
}

auto constexpr static maxCount = 100000000;

TEST(ItchReader, SkipAll) {
  auto const file = getTestFile();
  auto reader = md::BinaryDataReader(file.data(), file.size());
  auto count = size_t(0);

  while (count != maxCount && reader.remaining() > 3) {
    md::itch::skipCurrentMessage(reader);
    ++count;
  }
  ASSERT_EQ(count, maxCount);
}

TEST(ItchReader, CountMessageTypes) {
  auto const file = getTestFile();
  auto reader = md::BinaryDataReader(file.data(), file.size());
  auto count = size_t(0);

  auto counts = std::unordered_map<md::itch::messages::MessageType, size_t>();

  while (count != maxCount && reader.remaining() > 3) {
    ++counts[md::itch::currentMessageType(reader)];
    md::itch::skipCurrentMessage(reader);
    ++count;
  }

  for (auto const [messageType, c] : counts) {
    std::cout << static_cast<char>(messageType) << ": " << c << std::endl;
  }

  ASSERT_EQ(count, maxCount);
  ASSERT_EQ(counts[md::itch::messages::MessageType::ADD_ORDER], 43762947);
  ASSERT_EQ(counts[md::itch::messages::MessageType::ADD_ORDER_MPID], 1051447);
  ASSERT_EQ(counts[md::itch::messages::MessageType::EXECUTE_ORDER], 1804131);
  ASSERT_EQ(counts[md::itch::messages::MessageType::EXECUTE_ORDER_WITH_PRICE], 38458);
  ASSERT_EQ(counts[md::itch::messages::MessageType::REDUCE_ORDER], 2149719);
  ASSERT_EQ(counts[md::itch::messages::MessageType::DELETE_ORDER], 41720703);
  ASSERT_EQ(counts[md::itch::messages::MessageType::REPLACE_ORDER], 7840455);
}

TEST(ItchReader, TopNStocksByAdd) {
  auto const file = getTestFile();
  auto reader = md::BinaryDataReader(file.data(), file.size());

  auto stockMap = std::vector<std::string>(std::numeric_limits<uint16_t>::max());
  auto securityCount = std::unordered_map<uint16_t, size_t>();
  size_t startOfMessagesMsgId = 999999;
  size_t startOfSystemHoursMsgId = 999999;
  size_t startOfMarketHours = 999999;

  for (size_t msgi=0; msgi != maxCount; ++msgi) {
    auto const currentMessageType = md::itch::currentMessageType(reader);
    switch (currentMessageType) {
      case md::itch::messages::MessageType::SYSEVENT: {
        auto const msg = md::itch::readItchMessage<md::itch::messages::MessageType::SYSEVENT>(reader);
        std::cout << "message " << msgi << " - SYSEVENT: " << HHMM(msg.timeStamp) << ", " << msg.eventCode << std::endl;
        if (msg.eventCode == 'O') startOfMessagesMsgId = msgi;
        else if (msg.eventCode == 'S') startOfSystemHoursMsgId = msgi;
        else if (msg.eventCode == 'Q') startOfMarketHours = msgi;
        
        break;
      }
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
    }
  }

  ASSERT_EQ(startOfMessagesMsgId, 0);
  ASSERT_EQ(startOfSystemHoursMsgId, 219799);
  ASSERT_EQ(startOfMarketHours, 10532163);
  
  ASSERT_EQ(stockMap[6449], "QQQ     "s);
  ASSERT_EQ(stockMap[7291], "SPY     "s);
  ASSERT_EQ(stockMap[331],  "AMD     "s);
  ASSERT_EQ(stockMap[4260], "IWM     "s);
  ASSERT_EQ(stockMap[14],   "AAPL    "s);

  auto const topSecurityCount = [&]{
    //auto vecPairs = std::ranges::to<std::vector<std::pair<uint16_t, size_t>>>(securityCount);
    auto vecPairs = std::vector<std::pair<uint16_t, size_t>>();
    std::ranges::transform(securityCount, std::back_inserter(vecPairs), std::identity{});
    std::ranges::sort(vecPairs, [](auto const& lhs, auto const& rhs) { return lhs.second > rhs.second; });
    return vecPairs;
  }();
  
  int constexpr static N = 10;
  std::cout << "Top " << N << ": " << std::endl;
  for (auto i : std::views::iota(0, N)) {
    auto const [id, c] = topSecurityCount[i];
    std::cout << std::setw(5) << id << " (" << stockMap[id] << ")" << ": " << c << std::endl;
  }

  ASSERT_EQ(topSecurityCount[0], (std::pair<uint16_t, size_t>(6449, 487300)));
  ASSERT_EQ(topSecurityCount[1], (std::pair<uint16_t, size_t>(7291, 390022)));
  ASSERT_EQ(topSecurityCount[2], (std::pair<uint16_t, size_t>(331, 375023)));
  ASSERT_EQ(topSecurityCount[3], (std::pair<uint16_t, size_t>(4260, 284912)));
  ASSERT_EQ(topSecurityCount[4], (std::pair<uint16_t, size_t>(14, 271383)));
}

}  // namespace
