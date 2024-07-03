#pragma once

#include <chrono>

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

#include <cstdint>

namespace md::itch::types {

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

enum class BUY_SELL : char { BUY = 'B',
                             SELL = 'S' };
struct timestamp_t : std::chrono::nanoseconds {
  explicit timestamp_t(std::chrono::nanoseconds ns) : std::chrono::nanoseconds{ns} {};
  explicit timestamp_t(uint64_t ns) : std::chrono::nanoseconds{ns} {};
};
enum class oid_t : uint64_t {};
enum class price_t : uint32_t {};
enum class qty_t : uint32_t {};
using locate_t = uint16_t;

}  // namespace md::itch::types
