#pragma once

#include "magic.hpp"
#include "types.hpp"
#include <array>
#include <cstdint>

inline constexpr std::array<Bitboard, 8> file_masks = []() {
  std::array<Bitboard, 8> file_masks;

  for (uint8_t file = 0; file < 8; ++file)
    file_masks[file] = 0x0101010101010101ULL << file;

  return file_masks;
}();

inline constexpr std::array<Bitboard, 8> rank_masks = []() {
  std::array<Bitboard, 8> rank_masks;

  for (uint8_t rank = 0; rank < 8; ++rank)
    rank_masks[rank] = 0xFFULL << (8 * rank);

  return rank_masks;
}();

template <Piece> inline constexpr std::array<Bitboard, 64> pseudoattacks{};

template <>
inline constexpr std::array<Bitboard, 64> pseudoattacks<KNIGHT> = []() {
  std::array<Bitboard, 64> knight_moves;

  for (uint8_t pos = 0; pos < 64; ++pos) {
    Bitboard bitboard = 1ULL << pos;

    knight_moves[pos] =
        ((bitboard & ~(file_masks[0] | rank_masks[6] | rank_masks[7])) << 15) |
        ((bitboard & ~(file_masks[7] | rank_masks[6] | rank_masks[7])) << 17) |
        ((bitboard & ~(file_masks[7] | rank_masks[0] | rank_masks[1])) >> 15) |
        ((bitboard & ~(file_masks[0] | rank_masks[0] | rank_masks[1])) >> 17) |
        ((bitboard & ~(rank_masks[7] | file_masks[0] | file_masks[1])) << 6) |
        ((bitboard & ~(rank_masks[0] | file_masks[0] | file_masks[1])) >> 10) |
        ((bitboard & ~(rank_masks[0] | file_masks[6] | file_masks[7])) >> 6) |
        ((bitboard & ~(rank_masks[7] | file_masks[6] | file_masks[7])) << 10);
  }

  return knight_moves;
}();

template <>
inline constexpr std::array<Bitboard, 64> pseudoattacks<KING> = []() {
  std::array<Bitboard, 64> king_moves;

  for (uint8_t pos = 0; pos < 64; ++pos) {
    Bitboard bitboard = 1ULL << pos;
    king_moves[pos] = ((bitboard & ~(rank_masks[0] | file_masks[0])) >> 9) |
                      ((bitboard & ~(rank_masks[0] | file_masks[7])) >> 7) |
                      ((bitboard & ~(rank_masks[7] | file_masks[0])) << 7) |
                      ((bitboard & ~(rank_masks[7] | file_masks[7])) << 9) |
                      ((bitboard & ~file_masks[0]) >> 1) |
                      ((bitboard & ~file_masks[7]) << 1) |
                      ((bitboard & ~rank_masks[0]) >> 8) |
                      ((bitboard & ~rank_masks[7]) << 8);
  }

  return king_moves;
}();

inline constexpr std::array<std::array<Bitboard, 64>, NUM_SIDES> pawn_moves =
    []() {
      std::array<std::array<Bitboard, 64>, NUM_SIDES> pawn_moves = {};

      for (uint8_t pos = 0; pos < 64; ++pos) {
        Bitboard bitboard = 1ULL << pos;

        pawn_moves[WHITE][pos] =
            ((bitboard & ~(rank_masks[0] | rank_masks[7])) << 8) |
            ((bitboard & rank_masks[1]) << 16);

        pawn_moves[BLACK][pos] =
            ((bitboard & ~(rank_masks[0] | rank_masks[7])) >> 8) |
            ((bitboard & rank_masks[6]) >> 16);
      }

      return pawn_moves;
    }();

inline constexpr std::array<std::array<Bitboard, 64>, NUM_SIDES> pawn_attacks =
    []() {
      std::array<std::array<Bitboard, 64>, NUM_SIDES> pawn_attacks = {};

      for (uint8_t pos = 0; pos < 64; ++pos) {
        Bitboard bitboard = 1ULL << pos;

        pawn_attacks[WHITE][pos] =
            ((bitboard & ~(rank_masks[0] | rank_masks[7] | file_masks[0]))
             << 7) |
            ((bitboard & ~(rank_masks[0] | rank_masks[7] | file_masks[7]))
             << 9);

        pawn_attacks[BLACK][pos] =
            ((bitboard & ~(rank_masks[0] | rank_masks[7] | file_masks[0])) >>
             9) |
            ((bitboard & ~(rank_masks[0] | rank_masks[7] | file_masks[7])) >>
             7);
      }

      return pawn_attacks;
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

constexpr Bitboard isolate_rank(Bitboard bitboard, auto rank) {
  return bitboard & rank_masks[rank];
}

constexpr Bitboard get_bit(Square square) { return 1ULL << square; }

constexpr Square pop_lsb(Bitboard &bitboard) {
  Bitboard lsb = bitboard & -bitboard;
  bitboard &= ~lsb;
  return static_cast<Square>(1UZ << lsb);
}

void print_bitboard(Bitboard bitboard);
