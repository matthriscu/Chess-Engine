#pragma once

#include "enumarray.hpp"
#include <optional>

class Bitboard;
class Square;

enum class Direction {
  NORTH = 8,
  EAST = 1,
  SOUTH = -NORTH,
  WEST = -EAST,

  NORTH_WEST = NORTH + WEST,
  NORTH_EAST = NORTH + EAST,
  SOUTH_WEST = SOUTH + WEST,
  SOUTH_EAST = SOUTH + EAST
};

enum class Side : size_t { WHITE, BLACK };

inline constexpr size_t NUM_SIDES = 2;

enum class Piece : size_t { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

inline constexpr size_t NUM_PIECES = 6;

inline constexpr EnumArray<Piece, int, NUM_PIECES> PIECE_VALUES{1, 3, 3,
                                                                5, 9, 0};

constexpr Side opponent(Side side) {
  return side == Side::WHITE ? Side::BLACK : Side::WHITE;
}

constexpr char piece_to_char(std::optional<Piece> piece, Side side) {
  return piece
      .transform([&](Piece p) {
        return (side == Side::WHITE ? "PNBRQK"
                                    : "pnbrqk")[static_cast<size_t>(p)];
      })
      .value_or('.');
}

constexpr std::optional<Piece> char_to_piece(char c) {
  switch (c) {
  case 'P':
  case 'p':
    return Piece::PAWN;
  case 'N':
  case 'n':
    return Piece::KNIGHT;
  case 'B':
  case 'b':
    return Piece::BISHOP;
  case 'R':
  case 'r':
    return Piece::ROOK;
  case 'Q':
  case 'q':
    return Piece::QUEEN;
  case 'K':
  case 'k':
    return Piece::KING;
  default:
    return std::nullopt;
  }
}
