#pragma once

#include "constants.hpp"

constexpr Bitboard get_square_index(auto rank, auto file) {
  return 8 * rank + file;
}

constexpr Bitboard get_bit(auto rank, auto file) {
  return 1ULL << get_square_index(rank, file);
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

void print_bitboard(Bitboard bitboard);
