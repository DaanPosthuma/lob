#pragma once

#include "types.h"
#include <format>

template <>
struct std::formatter<md::itch::types::oid_t> {
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(md::itch::types::oid_t oid, FormatContext& ctx) const {
    return format_to(ctx.out(), "{}", static_cast<uint64_t>(oid));
  }
};
