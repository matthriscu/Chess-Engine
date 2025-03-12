#pragma once

#include <cstdint>

using Bitboard = uint64_t;

enum Side : uint8_t { WHITE, BLACK, NUM_SIDES, ALL_SIDES = NUM_SIDES };

constexpr Side opposite_side(Side side) {
  return side == WHITE ? BLACK : WHITE;
}

enum Piece : uint8_t {
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING,
  NUM_PIECES,
  ALL_PIECES = NUM_PIECES
};

enum Square : std::size_t {
  a1,
  a2,
  a3,
  a4,
  a5,
  a6,
  a7,
  a8,
  b1,
  b2,
  b3,
  b4,
  b5,
  b6,
  b7,
  b8,
  c1,
  c2,
  c3,
  c4,
  c5,
  c6,
  c7,
  c8,
  d1,
  d2,
  d3,
  d4,
  d5,
  d6,
  d7,
  d8,
  e1,
  e2,
  e3,
  e4,
  e5,
  e6,
  e7,
  e8,
  f1,
  f2,
  f3,
  f4,
  f5,
  f6,
  f7,
  f8,
  g1,
  g2,
  g3,
  g4,
  g5,
  g6,
  g7,
  g8,
  h1,
  h2,
  h3,
  h4,
  h5,
  h6,
  h7,
  h8
};

constexpr char piece_to_char(Piece piece, Side side) {
  if (piece < NUM_PIECES && side < NUM_SIDES)
    return (side == WHITE ? "PNBRQK" : "pnbrqk")[piece];
  else
    return '.';
}

constexpr Piece char_to_piece(char c) {
  switch (c) {
  case 'P':
  case 'p':
    return PAWN;
  case 'N':
  case 'n':
    return KNIGHT;
  case 'B':
  case 'b':
    return BISHOP;
  case 'R':
  case 'r':
    return ROOK;
  case 'Q':
  case 'q':
    return QUEEN;
  case 'K':
  case 'k':
    return KING;
  default:
    return NUM_PIECES;
  }
}
