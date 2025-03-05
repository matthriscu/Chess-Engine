#pragma once

#include "magic.hpp"
#include "types.hpp"
#include <array>
#include <cstdint>

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

template <Piece> inline constexpr std::array<Bitboard, 64> pseudoattacks{};

template <>
inline constexpr std::array<Bitboard, 64> pseudoattacks<KNIGHT> = []() {
  std::array<Bitboard, 64> moves;

  for (size_t square = 0; square < 64; ++square) {
    Bitboard bb = 1ULL << square;

    moves[square] = ((bb & ~(files[0] | ranks[6] | ranks[7])) << 15) |
                    ((bb & ~(files[7] | ranks[6] | ranks[7])) << 17) |
                    ((bb & ~(files[7] | ranks[0] | ranks[1])) >> 15) |
                    ((bb & ~(files[0] | ranks[0] | ranks[1])) >> 17) |
                    ((bb & ~(ranks[7] | files[0] | files[1])) << 6) |
                    ((bb & ~(ranks[0] | files[0] | files[1])) >> 10) |
                    ((bb & ~(ranks[0] | files[6] | files[7])) >> 6) |
                    ((bb & ~(ranks[7] | files[6] | files[7])) << 10);
  }

  return moves;
}();

template <>
inline constexpr std::array<Bitboard, 64> pseudoattacks<KING> = []() {
  std::array<Bitboard, 64> moves;

  for (size_t pos = 0; pos < 64; ++pos) {
    Bitboard bb = 1ULL << pos;

    moves[pos] = ((bb & ~(ranks[0] | files[0])) >> 9) |
                 ((bb & ~(ranks[0] | files[7])) >> 7) |
                 ((bb & ~(ranks[7] | files[0])) << 7) |
                 ((bb & ~(ranks[7] | files[7])) << 9) |
                 ((bb & ~files[0]) >> 1) | ((bb & ~files[7]) << 1) |
                 ((bb & ~ranks[0]) >> 8) | ((bb & ~ranks[7]) << 8);
  }

  return moves;
}();

inline constexpr std::array<std::array<Bitboard, 64>, NUM_SIDES> pawn_moves =
    []() {
      std::array<std::array<Bitboard, 64>, NUM_SIDES> moves = {};

      for (size_t square = 0; square < 64; ++square) {
        Bitboard bb = 1ULL << square;

        moves[WHITE][square] =
            ((bb & ~(ranks[0] | ranks[7])) << 8) | ((bb & ranks[1]) << 16);

        moves[BLACK][square] =
            ((bb & ~(ranks[0] | ranks[7])) >> 8) | ((bb & ranks[6]) >> 16);
      }

      return moves;
    }();

inline constexpr std::array<std::array<Bitboard, 64>, NUM_SIDES> pawn_attacks =
    []() {
      std::array<std::array<Bitboard, 64>, NUM_SIDES> attacks = {};

      for (size_t square = 0; square < 64; ++square) {
        Bitboard bb = 1ULL << square;

        attacks[WHITE][square] =
            ((bb & ~(ranks[0] | ranks[7] | files[0])) << 7) |
            ((bb & ~(ranks[0] | ranks[7] | files[7])) << 9);

        attacks[BLACK][square] =
            ((bb & ~(ranks[0] | ranks[7] | files[0])) >> 9) |
            ((bb & ~(ranks[0] | ranks[7] | files[7])) >> 7);
      }

      return attacks;
    }();

template <Piece P>
constexpr Bitboard attacks_bb(Square square, Bitboard occupied = 0) {
  switch (P) {
  case KNIGHT:
  case KING:
    return pseudoattacks<P>[square];
  case BISHOP:
  case ROOK:
  case QUEEN:
    return get_magic_attacks<P>(square, occupied);
  }
}

constexpr Bitboard attacks_bb(Piece p, Square square, Bitboard occupied = 0) {
  switch (p) {
  case KNIGHT:
    return attacks_bb<KNIGHT>(square);
  case KING:
    return attacks_bb<KING>(square);
  case BISHOP:
    return attacks_bb<BISHOP>(square, occupied);
  case ROOK:
    return attacks_bb<ROOK>(square, occupied);
  case QUEEN:
    return attacks_bb<QUEEN>(square, occupied);
  default:
    return 0;
  }
}

constexpr Square get_square(auto rank, auto file) {
  return static_cast<Square>(8UZ * rank + file);
}

constexpr Bitboard get_bit(auto rank, auto file) {
  return 1ULL << get_square(rank, file);
}

constexpr Bitboard get_bit(Square square) { return 1ULL << square; }

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
