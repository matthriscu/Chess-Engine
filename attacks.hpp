#pragma once

#include "bitboard.hpp"
#include "enumarray.hpp"
#include "magic.hpp"
#include "piece.hpp"
#include "types.hpp"

inline constexpr Sides::Array<Squares::Array<Bitboard>> pawn_attacks = []() {
  Sides::Array<Squares::Array<Bitboard>> attacks;

  for (Square square : Squares::ALL) {
    Bitboard bb(square);

    attacks[Sides::WHITE][square] =
        bb.shift<Direction::NORTH_WEST>() | bb.shift<Direction::NORTH_EAST>();

    attacks[Sides::BLACK][square] =
        bb.shift<Direction::SOUTH_WEST>() | bb.shift<Direction::SOUTH_EAST>();
  }

  return attacks;
}();

template <Piece::Literal P>
constexpr Bitboard attacks_bb(Square square, Bitboard occupied);

template <>
constexpr Bitboard attacks_bb<Pieces::KNIGHT>(Square square, Bitboard) {
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
constexpr Bitboard attacks_bb<Pieces::KING>(Square square, Bitboard) {
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
constexpr Bitboard attacks_bb<Pieces::BISHOP>(Square square,
                                              Bitboard occupied) {
  FancyHash m = magics<Pieces::BISHOP>[square.raw()];
  return m.attacks[((occupied.raw() | m.mask) * m.hash) >> (64 - 9)];
}

template <>
constexpr Bitboard attacks_bb<Pieces::ROOK>(Square square, Bitboard occupied) {
  FancyHash m = magics<Pieces::ROOK>[square.raw()];
  return m.attacks[((occupied.raw() | m.mask) * m.hash) >> (64 - 12)];
}

template <>
constexpr Bitboard attacks_bb<Pieces::QUEEN>(Square square, Bitboard occupied) {
  return attacks_bb<Pieces::ROOK>(square, occupied) |
         attacks_bb<Pieces::BISHOP>(square, occupied);
}

constexpr Bitboard attacks_bb(Piece p, Square square, Bitboard occupied = 0) {
  switch (p) {
  case Pieces::KNIGHT:
    return attacks_bb<Pieces::KNIGHT>(square, occupied);
  case Pieces::KING:
    return attacks_bb<Pieces::KING>(square, occupied);
  case Pieces::BISHOP:
    return attacks_bb<Pieces::BISHOP>(square, occupied);
  case Pieces::ROOK:
    return attacks_bb<Pieces::ROOK>(square, occupied);
  case Pieces::QUEEN:
    return attacks_bb<Pieces::QUEEN>(square, occupied);
  default:
    return 0;
  }
}
