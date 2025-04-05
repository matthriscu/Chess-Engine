#pragma once

#include "side.hpp"
#include <print>

class Piece {
public:
  enum class Literal { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NONE };

  constexpr Piece() : Piece(Literal::NONE) {}

  constexpr Piece(Literal data) : data(data) {}

  constexpr Piece(char c) {
    switch (c) {
    case 'P':
    case 'p':
      data = Literal::PAWN;
      break;
    case 'N':
    case 'n':
      data = Literal::KNIGHT;
      break;
    case 'B':
    case 'b':
      data = Literal::BISHOP;
      break;
    case 'R':
    case 'r':
      data = Literal::ROOK;
      break;
    case 'Q':
    case 'q':
      data = Literal::QUEEN;
      break;
    case 'K':
    case 'k':
      data = Literal::KING;
      break;
    default:
      data = Literal::NONE;
    }
  }

  constexpr operator Literal() const { return data; }

  constexpr char repr(Side side = Sides::BLACK) const {
    return (side == Sides::WHITE ? "PNBRQK." : "pnbrqk.")[raw()];
  }

  constexpr int raw() const { return static_cast<int>(data); }

private:
  Literal data;
};

namespace Pieces {
using enum Piece::Literal;

inline constexpr int NUM = 6;

inline constexpr std::array<Piece, NUM> ALL{PAWN, KNIGHT, BISHOP,
                                            ROOK, QUEEN,  KING};

template <typename T> using Array = EnumArray<Piece::Literal, T, NUM>;
} // namespace Pieces

template <> struct std::formatter<Piece> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(Piece piece, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{}", piece.repr());
  }
};
