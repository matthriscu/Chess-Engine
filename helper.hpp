#pragma once

#include <cstdint>

using Bitboard = uint64_t;

Bitboard get_square_index(auto rank, auto file) { return 8 * rank + file; }

Bitboard get_bit(auto rank, auto file) {
  return 1ULL << get_square_index(rank, file);
}

void print_bitboard(Bitboard bitboard);

Bitboard select_file(uint8_t file);

Bitboard select_rank(uint8_t rank);
