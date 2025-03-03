#include "piece.hpp"
#include "side.hpp"

char piece_to_char(Piece piece, Side side) {
  return (side == WHITE ? "PNBRQK" : "pnbrqk")[piece];
}

Piece char_to_piece(char c) {
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
    return ALL_PIECES;
  }
}
