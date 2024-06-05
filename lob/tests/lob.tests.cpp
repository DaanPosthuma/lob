#include <gtest/gtest.h>
#include <lob/RingBuffer.h>
#include <lob/lob.h>

namespace {
  
static_assert(lob::OrderId{12345} == lob::OrderId{12345});
static_assert(lob::OrderId{10} < lob::OrderId{50});

static_assert(lob::Level<4>{10000} == lob::Level<4>{10000});
static_assert(lob::Level<4>{500} < lob::Level<4>{10000});

TEST(LOB, AddAndDelete) {
  auto static constexpr Precision = 4;
  auto book = lob::LimitOrderBook<Precision>();
  using Level = lob::Level<Precision>;

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

  /*std::cout << "bid: " << book.bid() << std::endl;
  std::cout << "ask: " << book.ask() << std::endl;

  std::cout << "bid depth: " << book.bidDepth() << std::endl;
  std::cout << "ask depth: " << book.askDepth() << std::endl;

  std::cout << book << std::endl;

  std::cout << "deleting order " << id << std::endl;*/
  book.deleteOrder(id0);

  ASSERT_EQ(static_cast<int>(book.bid()), 230900);
  ASSERT_EQ(static_cast<int>(book.ask()), 231200);
  ASSERT_FLOAT_EQ(static_cast<double>(book.bid()), 23.09);
  ASSERT_FLOAT_EQ(static_cast<double>(book.ask()), 23.12);
  ASSERT_EQ(book.bidDepth(), 100);
  ASSERT_EQ(book.askDepth(), 250);

  book.deleteOrder(id1);

  ASSERT_EQ(static_cast<int>(book.bid()), 230800);
  ASSERT_EQ(static_cast<int>(book.ask()), 231200);
  ASSERT_FLOAT_EQ(static_cast<double>(book.bid()), 23.08);
  ASSERT_FLOAT_EQ(static_cast<double>(book.ask()), 23.12);
  ASSERT_EQ(book.bidDepth(), 200);
  ASSERT_EQ(book.askDepth(), 250);

  // std::cout << book << std::endl;

  /*book.PlaceMarketOrder(lob::Direction::Sell, 270);
  book.PlaceMarketOrder(lob::Direction::Buy, 560);*/
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
    ASSERT_EQ(data, (std::deque{0,1,2,3}));
  }

  {
    auto const [data, m, M] = b.readWithAsyncF(0, [&]{b.push(4);});

    ASSERT_EQ(data.size(), 4);
    ASSERT_EQ(m, 1);
    ASSERT_EQ(M, 4);
    ASSERT_EQ(data, (std::deque{1,2,3,4}));
  }

  {
    auto const [data, m, M] = b.readWithAsyncF(0, [&]{b.push(5); b.push(6);});

    ASSERT_EQ(data.size(), 4);
    ASSERT_EQ(m, 3);
    ASSERT_EQ(M, 6);
    ASSERT_EQ(data, (std::deque{3,4,5,6}));
  }
}

}  // namespace
