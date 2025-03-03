#pragma once

#include <cstdint>

using Bitboard = uint64_t;

constexpr Bitboard get_square_index(auto rank, auto file) {
  return 8 * rank + file;
}

constexpr Bitboard get_bit(auto rank, auto file) {
  return 1ULL << get_square_index(rank, file);
}

void print_bitboard(Bitboard bitboard);
