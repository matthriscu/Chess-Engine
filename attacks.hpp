#pragma once

#include "bitboard.hpp"
#include "enumarray.hpp"
#include "magic.hpp"
#include "types.hpp"

inline constexpr EnumArray<Side, EnumArray<Square, Bitboard, NUM_SQUARES>,
                           NUM_SIDES>
    pawn_attacks = []() {
      EnumArray<Side, EnumArray<Square, Bitboard, NUM_SQUARES>, NUM_SIDES>
          attacks;

      for (Square square : ALL_SQUARES) {
        Bitboard bb(square);

        attacks[Side::WHITE][square] = bb.shift<Direction::NORTH_WEST>() |
                                       bb.shift<Direction::NORTH_EAST>();

        attacks[Side::BLACK][square] = bb.shift<Direction::SOUTH_WEST>() |
                                       bb.shift<Direction::SOUTH_EAST>();
      }

      return attacks;
    }();

template <Piece P>
constexpr Bitboard attacks_bb(Square square, Bitboard occupied);

template <>
constexpr Bitboard attacks_bb<Piece::KNIGHT>(Square square, Bitboard) {
  static constexpr EnumArray<Square, Bitboard, NUM_SQUARES> pseudoattacks =
      []() {
        EnumArray<Square, Bitboard, NUM_SQUARES> moves;

        for (Square square : ALL_SQUARES) {
          Bitboard bb(square);

          moves[square] =
              bb.shift<Direction::NORTH>().shift<Direction::NORTH_WEST>() |
              bb.shift<Direction::NORTH>().shift<Direction::NORTH_EAST>() |
              bb.shift<Direction::WEST>().shift<Direction::NORTH_WEST>() |
              bb.shift<Direction::WEST>().shift<Direction::SOUTH_WEST>() |
              bb.shift<Direction::SOUTH>().shift<Direction::SOUTH_WEST>() |
              bb.shift<Direction::SOUTH>().shift<Direction::SOUTH_EAST>() |
              bb.shift<Direction::EAST>().shift<Direction::NORTH_EAST>() |
              bb.shift<Direction::EAST>().shift<Direction::SOUTH_EAST>();
        }

        return moves;
      }();

  return pseudoattacks[square];
}

template <>
constexpr Bitboard attacks_bb<Piece::KING>(Square square, Bitboard) {
  static constexpr EnumArray<Square, Bitboard, NUM_SQUARES> pseudoattacks =
      []() {
        EnumArray<Square, Bitboard, NUM_SQUARES> moves;

        for (Square square : ALL_SQUARES) {
          Bitboard bb(square);

          moves[square] =
              bb.shift<Direction::NORTH_WEST>() | bb.shift<Direction::NORTH>() |
              bb.shift<Direction::NORTH_EAST>() | bb.shift<Direction::WEST>() |
              bb.shift<Direction::SOUTH_WEST>() | bb.shift<Direction::SOUTH>() |
              bb.shift<Direction::SOUTH_EAST>() | bb.shift<Direction::EAST>();
        }

        return moves;
      }();

  return pseudoattacks[square];
}

template <>
constexpr Bitboard attacks_bb<Piece::BISHOP>(Square square, Bitboard occupied) {
  FancyHash m = magics<Piece::BISHOP>[static_cast<size_t>(square)];
  return m.attacks[((occupied.raw() | m.mask) * m.hash) >> (64 - 9)];
}

template <>
constexpr Bitboard attacks_bb<Piece::ROOK>(Square square, Bitboard occupied) {
  FancyHash m = magics<Piece::ROOK>[static_cast<size_t>(square)];
  return m.attacks[((occupied.raw() | m.mask) * m.hash) >> (64 - 12)];
}

template <>
constexpr Bitboard attacks_bb<Piece::QUEEN>(Square square, Bitboard occupied) {
  return attacks_bb<Piece::ROOK>(square, occupied) |
         attacks_bb<Piece::BISHOP>(square, occupied);
}

constexpr Bitboard attacks_bb(Piece p, Square square, Bitboard occupied = 0) {
  switch (p) {
  case Piece::KNIGHT:
    return attacks_bb<Piece::KNIGHT>(square, occupied);
  case Piece::KING:
    return attacks_bb<Piece::KING>(square, occupied);
  case Piece::BISHOP:
    return attacks_bb<Piece::BISHOP>(square, occupied);
  case Piece::ROOK:
    return attacks_bb<Piece::ROOK>(square, occupied);
  case Piece::QUEEN:
    return attacks_bb<Piece::QUEEN>(square, occupied);
  default:
    return 0;
  }
}
