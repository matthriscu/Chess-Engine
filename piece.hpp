#pragma once

#include "enumarray.hpp"
#include "side.hpp"
#include <print>

class Piece {
public:
  enum class Literal { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NONE };

  constexpr Piece() : Piece(Literal::NONE) {}

  constexpr Piece(Literal data) : data(data) {}

  constexpr Piece(int data) : data(static_cast<Literal>(data)) {}

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

  constexpr char repr(Side side = Sides::WHITE) const {
    return side == Sides::WHITE ? "PNBRQK."[raw()] : "pnbrqk."[raw()];
  }

  constexpr int raw() const { return static_cast<int>(data); }

  constexpr int value() const {
    switch (data) {
    case Literal::PAWN:
      return 1;
    case Literal::KNIGHT:
      return 3;
    case Literal::BISHOP:
      return 3;
    case Literal::ROOK:
      return 5;
    case Literal::QUEEN:
      return 9;
    default:
      return 0;
    }
  }

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
