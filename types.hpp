#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

using Bitboard = uint64_t;

// clang-format off
enum class Square : std::size_t {
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8,
};
// clang-format on

inline constexpr size_t NUM_SQUARES = 64;

inline constexpr std::array<Square, NUM_SQUARES> ALL_SQUARES = []() {
  std::array<Square, NUM_SQUARES> squares;

  std::ranges::generate(
      squares, [index = 0]() mutable { return static_cast<Square>(index++); });

  return squares;
}();

enum class Side : size_t { WHITE, BLACK };

inline constexpr size_t NUM_SIDES = 2;

enum class Piece : size_t { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

inline constexpr size_t NUM_PIECES = 6;

constexpr Side opponent(Side side) {
  return side == Side::WHITE ? Side::BLACK : Side::WHITE;
}

constexpr std::optional<Square> from_string(std::string_view str) {
  if (str.size() != 2 || !('a' <= str[0] && str[0] <= 'h') ||
      !('1' <= str[1] && str[1] <= '8'))
    return std::nullopt;

  return std::make_optional(
      static_cast<Square>((str[1] - '1') * 8 + (str[0] - 'a')));
}

constexpr std::string to_string(Square square) {
  return std::string(1, 'a' + static_cast<size_t>(square) % 8) +
         std::to_string(1 + static_cast<size_t>(square) / 8);
}

constexpr Square north(Square square) {
  return static_cast<Square>(static_cast<size_t>(square) + 8);
}

constexpr Square west(Square square) {
  return static_cast<Square>(static_cast<size_t>(square) - 1);
}

constexpr Square east(Square square) {
  return static_cast<Square>(static_cast<size_t>(square) + 1);
}

constexpr Square south(Square square) {
  return static_cast<Square>(static_cast<size_t>(square) - 8);
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
