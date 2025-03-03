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

char piece_to_char(Piece piece, Side side = WHITE);
Piece char_to_piece(char c);
