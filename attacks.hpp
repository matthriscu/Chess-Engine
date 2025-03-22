#pragma once

#include "bitboard.hpp"
#include "enumarray.hpp"
#include "magic.hpp"
#include "types.hpp"

inline constexpr EnumArray<Side, Squares::Array<Bitboard>, NUM_SIDES>
    pawn_attacks = []() {
      EnumArray<Side, Squares::Array<Bitboard>, NUM_SIDES> attacks;

      for (Square square : Squares::ALL) {
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
  static constexpr Squares::Array<Bitboard> pseudoattacks = []() {
    Squares::Array<Bitboard> moves;

    for (Square square : Squares::ALL) {
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
  static constexpr Squares::Array<Bitboard> pseudoattacks = []() {
    Squares::Array<Bitboard> moves;

    for (Square square : Squares::ALL) {
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
  FancyHash m = magics<Piece::BISHOP>[square.raw()];
  return m.attacks[((occupied.raw() | m.mask) * m.hash) >> (64 - 9)];
}

template <>
constexpr Bitboard attacks_bb<Piece::ROOK>(Square square, Bitboard occupied) {
  FancyHash m = magics<Piece::ROOK>[square.raw()];
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
