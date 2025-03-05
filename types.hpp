#pragma once

#include <cstdint>
#include <string>
#include <string_view>

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
  b1,
  c1,
  d1,
  e1,
  f1,
  g1,
  h1,
  a2,
  b2,
  c2,
  d2,
  e2,
  f2,
  g2,
  h2,
  a3,
  b3,
  c3,
  d3,
  e3,
  f3,
  g3,
  h3,
  a4,
  b4,
  c4,
  d4,
  e4,
  f4,
  g4,
  h4,
  a5,
  b5,
  c5,
  d5,
  e5,
  f5,
  g5,
  h5,
  a6,
  b6,
  c6,
  d6,
  e6,
  f6,
  g6,
  h6,
  a7,
  b7,
  c7,
  d7,
  e7,
  f7,
  g7,
  h7,
  a8,
  b8,
  c8,
  d8,
  e8,
  f8,
  g8,
  h8,
  NUM_SQUARES
};

constexpr Square from_string(std::string_view str) {
  if (str.size() != 2 || !('a' <= str[0] && str[0] <= 'h') ||
      !('1' <= str[1] && str[1] <= '8'))
    return NUM_SQUARES;
  return static_cast<Square>((str[1] - '1') * 8 + (str[0] - 'a'));
}

constexpr std::string to_string(Square square) {
  if (square == NUM_SQUARES)
    return "-";

  return std::string(1, 'a' + square % 8) + std::to_string(1 + square / 8);
}

constexpr Square north(Square square) {
  return static_cast<Square>(square + 8);
}

constexpr Square west(Square square) { return static_cast<Square>(square - 1); }

constexpr Square east(Square square) { return static_cast<Square>(square + 1); }

constexpr Square south(Square square) {
  return static_cast<Square>(square - 8);
}

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
