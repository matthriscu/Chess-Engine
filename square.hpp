#pragma once

#include "enumarray.hpp"
#include "types.hpp"
#include <algorithm>
#include <print>
#include <string>

class Square {
public:
  // clang-format off
  enum class Literal {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NONE
  };
  // clang-format on

  constexpr Square() : Square(Literal::NONE) {}

  constexpr Square(Literal data) : data(data) {}

  explicit constexpr Square(int data) : data(static_cast<Literal>(data)) {}

  constexpr Square(int rank, int file) : Square(8 * rank + file) {}

  constexpr Square(std::string_view str)
      : Square(str == "-" ? Square(Literal::NONE)
                          : Square(str[1] - '1', str[0] - 'a')) {}

  constexpr int raw() const { return static_cast<int>(data); }

  constexpr operator Literal() const { return data; }

  constexpr std::string to_string() const {
    return std::string(1, 'a' + raw() % 8) + std::to_string(1 + raw() / 8);
  }

  constexpr int rank() const { return raw() / 8; }

  constexpr int file() const { return raw() % 8; }

  template <Direction D> constexpr Square shift() const {
    switch (D) {
    case Direction::NORTH:
    case Direction::SOUTH:
      return Square(raw() + static_cast<int>(D));
    case Direction::WEST:
    case Direction::NORTH_WEST:
    case Direction::SOUTH_WEST:
      return Square(file() != 0 ? raw() + static_cast<int>(D) : raw());
    case Direction::EAST:
    case Direction::NORTH_EAST:
    case Direction::SOUTH_EAST:
      return Square(file() != 7 ? raw() + static_cast<int>(D) : raw());
    default:
      return Literal::NONE;
    };
  }

  constexpr Square shift(Direction d) const {
    switch (d) {
    case Direction::NORTH:
      return shift<Direction::NORTH>();
    case Direction::EAST:
      return shift<Direction::EAST>();
    case Direction::SOUTH:
      return shift<Direction::SOUTH>();
    case Direction::WEST:
      return shift<Direction::WEST>();
    case Direction::NORTH_WEST:
      return shift<Direction::NORTH_WEST>();
    case Direction::NORTH_EAST:
      return shift<Direction::NORTH_EAST>();
    case Direction::SOUTH_WEST:
      return shift<Direction::SOUTH_WEST>();
    case Direction::SOUTH_EAST:
      return shift<Direction::SOUTH_EAST>();
    default:
      return Literal::NONE;
    }
  }

private:
  Literal data;
};

namespace Squares {
using enum Square::Literal;

inline constexpr int NUM = 64;

inline constexpr std::array<Square, NUM> ALL = []() {
  std::array<Square, NUM> squares;

  std::generate_n(squares.begin(), NUM,
                  [i = 0]() mutable { return Square(i++); });

  return squares;
}();

template <typename T> using Array = EnumArray<Square::Literal, T, NUM>;
}; // namespace Squares

template <> struct std::formatter<Square> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(Square square, std::format_context &ctx) const {
    return square != Squares::NONE
               ? std::format_to(ctx.out(), "{:c}{}", 'a' + square.raw() % 8,
                                1 + square.raw() / 8)
               : std::format_to(ctx.out(), "-");
  }
};
