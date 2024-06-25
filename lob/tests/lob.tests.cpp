#include <gtest/gtest.h>
#include <lob/RingBuffer.h>
#include <lob/lob.h>

namespace {

static_assert(lob::PrecisionMultiplier<0>::value == 1.0000);
static_assert(lob::PrecisionMultiplier<1>::value == 0.1000);
static_assert(lob::PrecisionMultiplier<2>::value == 0.0100);
static_assert(lob::PrecisionMultiplier<3>::value == 0.0010);
static_assert(lob::PrecisionMultiplier<4>::value == 0.0001);

static_assert(lob::OrderId{12345} == lob::OrderId{12345});
static_assert(lob::OrderId{10} < lob::OrderId{50});

static_assert(lob::Level<4>{10000} == lob::Level<4>{10000});
static_assert(lob::Level<4>{500} < lob::Level<4>{10000});

static_assert(static_cast<double>(lob::Level<4>{10000}) == 1.0);
static_assert(static_cast<double>(lob::Level<4>{1}) == 0.0001);

TEST(LOB, AddAndDelete) {
  auto book = lob::LimitOrderBook();
  using Level = lob::LimitOrderBook::LevelT;

  book.addOrder(lob::Direction::Sell, 100, Level(231400));
  book.addOrder(lob::Direction::Sell, 100, Level(231400));
  book.addOrder(lob::Direction::Sell, 50, Level(231300));
  book.addOrder(lob::Direction::Sell, 100, Level(231300));
  book.addOrder(lob::Direction::Sell, 100, Level(231200));
  book.addOrder(lob::Direction::Sell, 50, Level(231200));
  book.addOrder(lob::Direction::Sell, 100, Level(231200));

  auto const id0 = book.addOrder(lob::Direction::Buy, 100, Level(230900));
  auto const id1 = book.addOrder(lob::Direction::Buy, 100, Level(230900));
  book.addOrder(lob::Direction::Buy, 50, Level(230800));
  book.addOrder(lob::Direction::Buy, 100, Level(230800));
  book.addOrder(lob::Direction::Buy, 50, Level(230800));
  book.addOrder(lob::Direction::Buy, 100, Level(230700));
  book.addOrder(lob::Direction::Buy, 100, Level(230700));

  ASSERT_EQ(static_cast<int>(book.bid()), 230900);
  ASSERT_EQ(static_cast<int>(book.ask()), 231200);
  ASSERT_FLOAT_EQ(static_cast<double>(book.bid()), 23.09);
  ASSERT_FLOAT_EQ(static_cast<double>(book.ask()), 23.12);
  ASSERT_EQ(book.bidDepth(), 200);
  ASSERT_EQ(book.askDepth(), 250);

  ASSERT_TRUE(book.deleteOrder(id0));

  ASSERT_EQ(static_cast<int>(book.bid()), 230900);
  ASSERT_EQ(static_cast<int>(book.ask()), 231200);
  ASSERT_FLOAT_EQ(static_cast<double>(book.bid()), 23.09);
  ASSERT_FLOAT_EQ(static_cast<double>(book.ask()), 23.12);
  ASSERT_EQ(book.bidDepth(), 100);
  ASSERT_EQ(book.askDepth(), 250);

  ASSERT_TRUE(book.deleteOrder(id1));

  ASSERT_EQ(static_cast<int>(book.bid()), 230800);
  ASSERT_EQ(static_cast<int>(book.ask()), 231200);
  ASSERT_FLOAT_EQ(static_cast<double>(book.bid()), 23.08);
  ASSERT_FLOAT_EQ(static_cast<double>(book.ask()), 23.12);
  ASSERT_EQ(book.bidDepth(), 200);
  ASSERT_EQ(book.askDepth(), 250);
}

TEST(LOB, AddAndReplace) {
  auto book = lob::LimitOrderBook();
  using Level = lob::LimitOrderBook::LevelT;

  auto id0 = book.addOrder(lob::Direction::Sell, 100, Level(1010000));
  auto id1 = book.addOrder(lob::Direction::Buy, 100, Level(990000));

  ASSERT_EQ(static_cast<int>(book.bid()), 990000);
  ASSERT_EQ(static_cast<int>(book.ask()), 1010000);
  ASSERT_EQ(static_cast<double>(book.bid()), 99.0);
  ASSERT_EQ(static_cast<double>(book.ask()), 101.0);
  ASSERT_EQ(book.bidDepth(), 100);
  ASSERT_EQ(book.askDepth(), 100);

  ASSERT_TRUE(book.replaceOrder(id0, id0, 50, Level(1010000)));
  ASSERT_TRUE(book.replaceOrder(id1, id1, 150, Level(980000)));

  ASSERT_EQ(static_cast<int>(book.bid()), 980000);
  ASSERT_EQ(static_cast<int>(book.ask()), 1010000);
  ASSERT_EQ(static_cast<double>(book.bid()), 98.0);
  ASSERT_EQ(static_cast<double>(book.ask()), 101.0);
  ASSERT_EQ(book.bidDepth(), 150);
  ASSERT_EQ(book.askDepth(), 50);
}

TEST(LOB, AddAndReduce) {
  auto book = lob::LimitOrderBook();
  using Level = lob::LimitOrderBook::LevelT;

  book.addOrder(lob::Direction::Sell, 100, Level(101));
  auto id0 = book.addOrder(lob::Direction::Sell, 100, Level(101));
  auto id1 = book.addOrder(lob::Direction::Buy, 100, Level(99));
  book.addOrder(lob::Direction::Buy, 300, Level(98));

  ASSERT_EQ(static_cast<int>(book.bid()), 99);
  ASSERT_EQ(static_cast<int>(book.ask()), 101);
  ASSERT_EQ(book.bidDepth(), 100);
  ASSERT_EQ(book.askDepth(), 200);

  ASSERT_TRUE(book.reduceOrder(id0, 20));
  ASSERT_TRUE(book.reduceOrder(id1, 60));

  ASSERT_EQ(static_cast<int>(book.bid()), 99);
  ASSERT_EQ(static_cast<int>(book.ask()), 101);
  ASSERT_EQ(book.bidDepth(), 40);
  ASSERT_EQ(book.askDepth(), 180);
}

TEST(LOB, AddAndExecute) {
  auto book = lob::LimitOrderBook();
  using Level = lob::LimitOrderBook::LevelT;

  book.addOrder(lob::Direction::Sell, 100, Level(231400));
  book.addOrder(lob::Direction::Sell, 100, Level(231400));
  book.addOrder(lob::Direction::Sell, 50, Level(231300));
  book.addOrder(lob::Direction::Sell, 100, Level(231300));
  book.addOrder(lob::Direction::Sell, 100, Level(231200));
  book.addOrder(lob::Direction::Sell, 50, Level(231200));
  book.addOrder(lob::Direction::Sell, 100, Level(231200));

  auto const id0 = book.addOrder(lob::Direction::Buy, 100, Level(230900));
  auto const id1 = book.addOrder(lob::Direction::Buy, 100, Level(230900));
  book.addOrder(lob::Direction::Buy, 50, Level(230800));
  book.addOrder(lob::Direction::Buy, 100, Level(230800));
  book.addOrder(lob::Direction::Buy, 50, Level(230800));
  book.addOrder(lob::Direction::Buy, 100, Level(230700));
  book.addOrder(lob::Direction::Buy, 100, Level(230700));

  ASSERT_EQ(static_cast<int>(book.bid()), 230900);
  ASSERT_EQ(static_cast<int>(book.ask()), 231200);
  ASSERT_EQ(book.bidDepth(), 200);
  ASSERT_EQ(book.askDepth(), 250);

  ASSERT_EQ(book.executeOrder(id0, 20), lob::ExecuteOrderResult::PARTIAL);

  ASSERT_EQ(static_cast<int>(book.bid()), 230900);
  ASSERT_EQ(static_cast<int>(book.ask()), 231200);
  ASSERT_EQ(book.bidDepth(), 180);
  ASSERT_EQ(book.askDepth(), 250);

  ASSERT_EQ(book.executeOrder(id0, 80), lob::ExecuteOrderResult::FULL);

  ASSERT_EQ(static_cast<int>(book.bid()), 230900);
  ASSERT_EQ(static_cast<int>(book.ask()), 231200);
  ASSERT_EQ(book.bidDepth(), 100);
  ASSERT_EQ(book.askDepth(), 250);

  ASSERT_EQ(book.executeOrder(id1, 50), lob::ExecuteOrderResult::PARTIAL);

  ASSERT_EQ(static_cast<int>(book.bid()), 230900);
  ASSERT_EQ(static_cast<int>(book.ask()), 231200);
  ASSERT_EQ(book.bidDepth(), 50);
  ASSERT_EQ(book.askDepth(), 250);

  ASSERT_EQ(book.executeOrder(id1, 50), lob::ExecuteOrderResult::FULL);

  ASSERT_EQ(static_cast<int>(book.bid()), 230800);
  ASSERT_EQ(static_cast<int>(book.ask()), 231200);
  ASSERT_EQ(book.bidDepth(), 200);
  ASSERT_EQ(book.askDepth(), 250);

}

TEST(LOB, RingBuffer) {
  auto b = RingBuffer<int, 4>();

  {
    auto const [data, m, M] = b.read(0);

    ASSERT_EQ(data.size(), 0);
    ASSERT_EQ(m, 0);
    ASSERT_EQ(M, 0);
  }

  b.push(0);

  {
    auto const [data, m, M] = b.read(0);

    ASSERT_EQ(data.size(), 1);
    ASSERT_EQ(m, 0);
    ASSERT_EQ(M, 0);
    ASSERT_EQ(data, (std::deque{0}));
  }

  b.push(1);

  {
    auto const [data, m, M] = b.read(0);

    ASSERT_EQ(data.size(), 2);
    ASSERT_EQ(m, 0);
    ASSERT_EQ(M, 1);
    ASSERT_EQ(data, (std::deque{0, 1}));
  }

  b.push(2);
  b.push(3);

  {
    auto const [data, m, M] = b.read(0);

    ASSERT_EQ(data.size(), 4);
    ASSERT_EQ(m, 0);
    ASSERT_EQ(M, 3);
    ASSERT_EQ(data, (std::deque{0, 1, 2, 3}));
  }

  {
    auto [data, m, M] = b.read(2);

    ASSERT_EQ(data.size(), 2);
    ASSERT_EQ(m, 2);
    ASSERT_EQ(M, 3);
    ASSERT_EQ(data, (std::deque{2, 3}));
  }

  b.push(4);

  {
    auto const [data, m, M] = b.read(0);

    ASSERT_EQ(data.size(), 4);
    ASSERT_EQ(m, 1);
    ASSERT_EQ(M, 4);
    ASSERT_EQ(data, (std::deque{1, 2, 3, 4}));
  }

  b.push(5);

  {
    auto const [data, m, M] = b.read(0);

    ASSERT_EQ(data.size(), 4);
    ASSERT_EQ(m, 2);
    ASSERT_EQ(M, 5);
    ASSERT_EQ(data, (std::deque{2, 3, 4, 5}));
  }

  b.push(6);

  {
    auto const [data, m, M] = b.read(0);

    ASSERT_EQ(data.size(), 4);
    ASSERT_EQ(m, 3);
    ASSERT_EQ(M, 6);
    ASSERT_EQ(data, (std::deque{3, 4, 5, 6}));
  }

  b.push(7);

  {
    auto const [data, m, M] = b.read(0);

    ASSERT_EQ(data.size(), 4);
    ASSERT_EQ(m, 4);
    ASSERT_EQ(M, 7);
    ASSERT_EQ(data, (std::deque{4, 5, 6, 7}));
  }

  b.push(8);

  {
    auto const [data, m, M] = b.read(0);

    ASSERT_EQ(data.size(), 4);
    ASSERT_EQ(m, 5);
    ASSERT_EQ(M, 8);
    ASSERT_EQ(data, (std::deque{5, 6, 7, 8}));
  }

  {
    auto const [data, m, M] = b.read(5);

    ASSERT_EQ(data.size(), 4);
    ASSERT_EQ(m, 5);
    ASSERT_EQ(M, 8);
    ASSERT_EQ(data, (std::deque{5, 6, 7, 8}));
  }

  {
    auto const [data, m, M] = b.read(6);

    ASSERT_EQ(data.size(), 3);
    ASSERT_EQ(m, 6);
    ASSERT_EQ(M, 8);
    ASSERT_EQ(data, (std::deque{6, 7, 8}));
  }

  {
    auto const [data, m, M] = b.read(7);

    ASSERT_EQ(data.size(), 2);
    ASSERT_EQ(m, 7);
    ASSERT_EQ(M, 8);
    ASSERT_EQ(data, (std::deque{7, 8}));
  }

  {
    auto const [data, m, M] = b.read(8);

    ASSERT_EQ(data.size(), 1);
    ASSERT_EQ(m, 8);
    ASSERT_EQ(M, 8);
    ASSERT_EQ(data, (std::deque{8}));
  }

  {
    auto const [data, m, M] = b.read(9);

    ASSERT_EQ(data.size(), 0);
    ASSERT_EQ(m, 0);
    ASSERT_EQ(M, 0);
  }
}

TEST(LOB, RingBufferAsync) {
  auto b = RingBuffer<int, 4>();

  b.push(0);
  b.push(1);
  b.push(2);
  b.push(3);

  {
    auto const [data, m, M] = b.read(0);

    ASSERT_EQ(data.size(), 4);
    ASSERT_EQ(m, 0);
    ASSERT_EQ(M, 3);
    ASSERT_EQ(data, (std::deque{0, 1, 2, 3}));
  }

  {
    auto const [data, m, M] = b.readWithAsyncF(0, [&] { b.push(4); });

    ASSERT_EQ(data.size(), 4);
    ASSERT_EQ(m, 1);
    ASSERT_EQ(M, 4);
    ASSERT_EQ(data, (std::deque{1, 2, 3, 4}));
  }

  {
    auto const [data, m, M] = b.readWithAsyncF(0, [&] {b.push(5); b.push(6); });

    ASSERT_EQ(data.size(), 4);
    ASSERT_EQ(m, 3);
    ASSERT_EQ(M, 6);
    ASSERT_EQ(data, (std::deque{3, 4, 5, 6}));
  }
}

}  // namespace
