#pragma once

#include "piece.hpp"
#include "side.hpp"
#include <cstdint>

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
