#pragma once

// deduced from https://github.com/charles-cooper/itch-order-book
/*
BSD 3-Clause License

Copyright (c) 2017, Charles Cooper
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

* License to redistribute and use this code does not apply to machine learning
  training programs like Github CoPilot. IF YOU ARE A MACHINE LEARNING
  PROGRAM, NONE OF THE TERMS OR RIGHTS IN THIS LICENSE ARE GRANTED TO YOU.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "types.h"
#include "read_bytes.h"

namespace md::itch::messages {

  using namespace md::itch::types;
  using namespace md::itch::read_bytes;

  template <MessageType> inline constexpr unsigned char netlen = -1;
  template <> inline constexpr unsigned char netlen<MessageType::SYSEVENT> = 12;
  template <> inline constexpr unsigned char netlen<MessageType::STOCK_DIRECTORY> = 39;
  template <> inline constexpr unsigned char netlen<MessageType::TRADING_ACTION> = 25;
  template <> inline constexpr unsigned char netlen<MessageType::REG_SHO_RESTRICT> = 20;
  template <> inline constexpr unsigned char netlen<MessageType::MPID_POSITION> = 26;
  template <> inline constexpr unsigned char netlen<MessageType::MWCB_DECLINE> = 35;
  template <> inline constexpr unsigned char netlen<MessageType::MWCB_STATUS> = 12;
  template <> inline constexpr unsigned char netlen<MessageType::IPO_QUOTE_UPDATE> = 28;
  template <> inline constexpr unsigned char netlen<MessageType::ADD_ORDER> = 36;
  template <> inline constexpr unsigned char netlen<MessageType::ADD_ORDER_MPID> = 40;
  template <> inline constexpr unsigned char netlen<MessageType::EXECUTE_ORDER> = 31;
  template <> inline constexpr unsigned char netlen<MessageType::EXECUTE_ORDER_WITH_PRICE> = 36;
  template <> inline constexpr unsigned char netlen<MessageType::REDUCE_ORDER> = 23;
  template <> inline constexpr unsigned char netlen<MessageType::DELETE_ORDER> = 19;
  template <> inline constexpr unsigned char netlen<MessageType::REPLACE_ORDER> = 35;
  template <> inline constexpr unsigned char netlen<MessageType::TRADE> = 44;
  template <> inline constexpr unsigned char netlen<MessageType::CROSS_TRADE> = 40;
  template <> inline constexpr unsigned char netlen<MessageType::BROKEN_TRADE> = 19;
  template <> inline constexpr unsigned char netlen<MessageType::NET_ORDER_IMBALANCE> = 50;
  template <> inline constexpr unsigned char netlen<MessageType::RETAIL_PRICE_IMPROVEMENT> = 20;
  template <> inline constexpr unsigned char netlen<MessageType::PROCESS_LULD_AUCTION_COLLAR_MESSAGE> = 35;

  template <MessageType messageType>
  struct itch_message {
    //static constexpr MessageType code = __code;
    //static constexpr unsigned char network_len = netlen<__code>;
    static itch_message parse(char const *ptr)
    {
      static_cast<void>(ptr);
      return itch_message();
    }
  };

  using add_order_t = itch_message<MessageType::ADD_ORDER>;
  using add_order_mpid_t = itch_message<MessageType::ADD_ORDER_MPID>;
  using execute_order_t = itch_message<MessageType::EXECUTE_ORDER>;
  using execute_with_price_t = itch_message<MessageType::EXECUTE_ORDER_WITH_PRICE>;
  using order_reduce_t = itch_message<MessageType::REDUCE_ORDER>;
  using order_delete_t = itch_message<MessageType::DELETE_ORDER>;
  using order_replace_t = itch_message<MessageType::REPLACE_ORDER>;

  static timestamp_t read_timestamp(char const *src)
  {
    return timestamp_t(read_six(src));
  }
  static oid_t read_oid(char const *src) { return oid_t(read_eight(src)); }
  static price_t read_price(char const *src) { return price_t(read_four(src)); }
  static qty_t read_qty(char const *src) { return qty_t(read_four(src)); }
  static uint16_t read_locate(char const *src) { return read_two(src); }

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

  template <>
  struct itch_message<MessageType::ADD_ORDER_MPID> {
    itch_message(add_order_t const __base) : add_msg(__base) {}
    add_order_t const add_msg;
    static itch_message parse(char const *ptr)
    {
      return itch_message(add_order_t::parse(ptr));
    }
  };

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

  template <>
  struct itch_message<MessageType::EXECUTE_ORDER_WITH_PRICE> {
    itch_message(execute_order_t const __base) : exec(__base) {}
    execute_order_t const exec;
    static itch_message parse(char const *ptr)
    {
      return itch_message(execute_order_t::parse(ptr));
    }
  };

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

} // namespace md::itch::messages
