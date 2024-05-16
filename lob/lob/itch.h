#pragma once
#include <sys/types.h>

#ifdef _WIN32

#include <cstdint>

inline constexpr uint64_t be64toh(uint64_t big_endian_value) noexcept {
    // Swap byte order manually
    return ((big_endian_value & 0x00000000000000FFULL) << 56) |
           ((big_endian_value & 0x000000000000FF00ULL) << 40) |
           ((big_endian_value & 0x0000000000FF0000ULL) << 24) |
           ((big_endian_value & 0x00000000FF000000ULL) << 8) |
           ((big_endian_value & 0x000000FF00000000ULL) >> 8) |
           ((big_endian_value & 0x0000FF0000000000ULL) >> 24) |
           ((big_endian_value & 0x00FF000000000000ULL) >> 40) |
           ((big_endian_value & 0xFF00000000000000ULL) >> 56);
}

inline constexpr uint32_t be32toh(uint32_t big_endian_value) noexcept {
    // Swap byte order manually
    return ((big_endian_value & 0x000000FFU) << 24) |
           ((big_endian_value & 0x0000FF00U) << 8) |
           ((big_endian_value & 0x00FF0000U) >> 8) |
           ((big_endian_value & 0xFF000000U) >> 24);
}

inline constexpr uint16_t be16toh(uint16_t big_endian_value) noexcept {
    // Swap byte order manually
    return ((big_endian_value & 0x00FFU) << 8) |
           ((big_endian_value & 0xFF00U) >> 8);
}

#endif

enum class MessageType {
  SYSEVENT = 'S',
  STOCK_DIRECTORY = 'R',
  TRADING_ACTION = 'H',
  REG_SHO_RESTRICT = 'Y',  // 20
  MPID_POSITION = 'L',     // 26
  MWCB_DECLINE = 'V',      // market wide circuit breaker // 35
  MWCB_STATUS = 'W',
  IPO_QUOTE_UPDATE = 'K',  // 28
  ADD_ORDER = 'A',         // 36
  ADD_ORDER_MPID = 'F',
  EXECUTE_ORDER = 'E',
  EXECUTE_ORDER_WITH_PRICE = 'C',
  REDUCE_ORDER = 'X',
  DELETE_ORDER = 'D',
  REPLACE_ORDER = 'U',
  TRADE = 'P',
  CROSS_TRADE = 'Q',
  BROKEN_TRADE = 'B',
  NET_ORDER_IMBALANCE = 'I',
  RETAIL_PRICE_IMPROVEMENT = 'N',
  PROCESS_LULD_AUCTION_COLLAR_MESSAGE = 'J'
};
template <MessageType> constexpr unsigned char netlen = -1;
template <> constexpr unsigned char netlen<MessageType::SYSEVENT> = 12;
template <> constexpr unsigned char netlen<MessageType::STOCK_DIRECTORY> = 39;
template <> constexpr unsigned char netlen<MessageType::TRADING_ACTION> = 25;
template <> constexpr unsigned char netlen<MessageType::REG_SHO_RESTRICT> = 20;
template <> constexpr unsigned char netlen<MessageType::MPID_POSITION> = 26;
template <> constexpr unsigned char netlen<MessageType::MWCB_DECLINE> = 35;
template <> constexpr unsigned char netlen<MessageType::MWCB_STATUS> = 12;
template <> constexpr unsigned char netlen<MessageType::IPO_QUOTE_UPDATE> = 28;
template <> constexpr unsigned char netlen<MessageType::ADD_ORDER> = 36;
template <> constexpr unsigned char netlen<MessageType::ADD_ORDER_MPID> = 40;
template <> constexpr unsigned char netlen<MessageType::EXECUTE_ORDER> = 31;
template <> constexpr unsigned char netlen<MessageType::EXECUTE_ORDER_WITH_PRICE> = 36;
template <> constexpr unsigned char netlen<MessageType::REDUCE_ORDER> = 23;
template <> constexpr unsigned char netlen<MessageType::DELETE_ORDER> = 19;
template <> constexpr unsigned char netlen<MessageType::REPLACE_ORDER> = 35;
template <> constexpr unsigned char netlen<MessageType::TRADE> = 44;
template <> constexpr unsigned char netlen<MessageType::CROSS_TRADE> = 40;
template <> constexpr unsigned char netlen<MessageType::BROKEN_TRADE> = 19;
template <> constexpr unsigned char netlen<MessageType::NET_ORDER_IMBALANCE> = 50;
template <> constexpr unsigned char netlen<MessageType::RETAIL_PRICE_IMPROVEMENT> = 20;
template <> constexpr unsigned char netlen<MessageType::PROCESS_LULD_AUCTION_COLLAR_MESSAGE> = 35;

template <MessageType __code>
struct itch_message {
  static constexpr MessageType code = __code;
  static constexpr unsigned char network_len = netlen<__code>;
  static itch_message parse(char const *ptr)
  {
    static_cast<void>(ptr);
    return itch_message();
  }
};

enum class BUY_SELL : char { BUY = 'B', SELL = 'S' };
enum class timestamp_t : uint64_t {};
enum class oid_t : uint64_t {};
enum class price_t : uint32_t {};
enum class qty_t : uint32_t {};
static uint64_t read_eight(char const *src)
{
  return be64toh(*(uint64_t const *)src);
}
static uint64_t read_six(char const *src)
{
  uint64_t ret;
  char *pun = (char *)&ret;
  // it's not clear whether this is faster than six separate char assignments
  std::memcpy(pun, src, 6);
  return (be64toh(ret) >> 16);
}
static uint32_t read_four(char const *src)
{
  return be32toh(*(uint32_t const *)src);
}
static uint16_t read_two(char const *src)
{
  return be16toh(*(uint16_t const *)src);
}

static timestamp_t read_timestamp(char const *src)
{
  return timestamp_t(read_six(src));
}
static oid_t read_oid(char const *src) { return oid_t(read_eight(src)); }
static price_t read_price(char const *src) { return price_t(read_four(src)); }
static qty_t read_qty(char const *src) { return qty_t(read_four(src)); }
static uint16_t read_locate(char const *src) { return read_two(src); }
using add_order_t = itch_message<MessageType::ADD_ORDER>;
template <>
struct itch_message<MessageType::ADD_ORDER> {
  itch_message(timestamp_t __timestamp, oid_t __oid, price_t __price,
               qty_t __qty, uint16_t __stock_locate, BUY_SELL __buy)
      : timestamp(__timestamp),
        oid(__oid),
        price(__price),
        qty(__qty),
        stock_locate(__stock_locate),
        buy(__buy)
  {
  }
  timestamp_t const timestamp;
  oid_t const oid;
  price_t const price;
  qty_t const qty;
  uint16_t const stock_locate;
  BUY_SELL const buy;
  static itch_message parse(char const *ptr)
  {
    return add_order_t(read_timestamp(ptr + 5), read_oid(ptr + 11),
                       read_price(ptr + 32), read_qty(ptr + 20),
                       read_locate(ptr + 1), BUY_SELL(*(ptr + 19)));
  }
};
using add_order_mpid_t = itch_message<MessageType::ADD_ORDER_MPID>;
template <>
struct itch_message<MessageType::ADD_ORDER_MPID> {
  itch_message(add_order_t const __base) : add_msg(__base) {}
  add_order_t const add_msg;
  static itch_message parse(char const *ptr)
  {
    return itch_message(add_order_t::parse(ptr));
  }
};
using execute_order_t = itch_message<MessageType::EXECUTE_ORDER>;
template <>
struct itch_message<MessageType::EXECUTE_ORDER> {
  itch_message(oid_t __oid, timestamp_t __t, qty_t __q, uint16_t __s)
      : oid(__oid), timestamp(__t), qty(__q), stock_locate(__s)
  {
  }
  oid_t const oid;
  timestamp_t const timestamp;
  qty_t const qty;
  uint16_t const stock_locate;
  static itch_message parse(char const *ptr)
  {
    return itch_message(read_oid(ptr + 11), read_timestamp(ptr + 5),
                        read_qty(ptr + 19), read_locate(ptr + 1));
  }
};
using execute_with_price_t = itch_message<MessageType::EXECUTE_ORDER_WITH_PRICE>;
template <>
struct itch_message<MessageType::EXECUTE_ORDER_WITH_PRICE> {
  itch_message(execute_order_t const __base) : exec(__base) {}
  execute_order_t const exec;
  static itch_message parse(char const *ptr)
  {
    return itch_message(execute_order_t::parse(ptr));
  }
};
using order_reduce_t = itch_message<MessageType::REDUCE_ORDER>;
template <>
struct itch_message<MessageType::REDUCE_ORDER> {
  itch_message(oid_t __o, timestamp_t __t, qty_t __q)
      : oid(__o), timestamp(__t), qty(__q)
  {
  }
  oid_t const oid;
  timestamp_t const timestamp;
  qty_t const qty;
  static itch_message parse(char const *ptr)
  {
    return itch_message(read_oid(ptr + 11), read_timestamp(ptr + 5),
                        read_qty(ptr + 19));
  }
};
using order_delete_t = itch_message<MessageType::DELETE_ORDER>;
template <>
struct itch_message<MessageType::DELETE_ORDER> {
  itch_message(oid_t __o, timestamp_t __t) : oid(__o), timestamp(__t) {}
  oid_t const oid;
  timestamp_t const timestamp;
  static itch_message parse(char const *ptr)
  {
    return itch_message(read_oid(ptr + 11), read_timestamp(ptr + 5));
  }
};
using order_replace_t = itch_message<MessageType::REPLACE_ORDER>;
template <>
struct itch_message<MessageType::REPLACE_ORDER> {
  itch_message(oid_t __old_oid, oid_t __new_oid, qty_t __q, price_t __p)
      : oid(__old_oid), new_order_id(__new_oid), new_qty(__q), new_price(__p)
  {
  }
  oid_t const oid;
  oid_t const new_order_id;
  qty_t const new_qty;
  price_t const new_price;
  static itch_message parse(char const *ptr)
  {
    return itch_message(read_oid(ptr + 11), read_oid(ptr + 19),
                        read_qty(ptr + 27), read_price(ptr + 31));
  }
};
