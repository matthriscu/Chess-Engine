#pragma once

#include "types.hpp"
#include <array>

inline constexpr std::array<Bitboard, 8> files = []() {
  std::array<Bitboard, 8> files;

  for (size_t file = 0; file < 8; ++file)
    files[file] = 0x0101010101010101ULL << file;

  return files;
}();

inline constexpr std::array<Bitboard, 8> ranks = []() {
  std::array<Bitboard, 8> ranks;

  for (size_t rank = 0; rank < 8; ++rank)
    ranks[rank] = 0xFFULL << (8 * rank);

  return ranks;
}();

constexpr Square get_square(auto rank, auto file) {
  return static_cast<Square>(8UZ * rank + file);
}

constexpr Bitboard get_bit(auto rank, auto file) {
  return get_bit(get_square(rank, file));
}

constexpr Bitboard get_bit(Square square) {
  return 1ULL << static_cast<size_t>(square);
}

constexpr Bitboard north(Bitboard bb) { return (bb & ~ranks[7]) << 8; }

constexpr Bitboard west(Bitboard bb) { return (bb & ~files[0]) << 1; }

constexpr Bitboard east(Bitboard bb) { return (bb & ~files[7]) >> 1; }

constexpr Bitboard south(Bitboard bb) { return (bb & ~ranks[0]) >> 8; }

constexpr Square pop_lsb(Bitboard &bb) {
  Bitboard lsb = bb & -bb;
  bb &= ~lsb;

  return static_cast<Square>(std::countr_zero(lsb));
}

void print_bitboard(Bitboard bitboard);
