#pragma once

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
