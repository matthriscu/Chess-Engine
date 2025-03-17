#include "types.hpp"
#include <cstdint>

enum class Special {
  DOUBLE_PUSH,
  KING_CASTLE,
  QUEEN_CASTLE,
  EN_PASSANT,
};

struct Move {
  // https://www.chessprogramming.org/Encoding_Moves#From-To_Based
  // P C S S T T T T T T F F F F F F

  static constexpr uint16_t flags = 0b1111000000000000;

  uint16_t data;

  constexpr Move() = default;

  constexpr Move(Square from, Square to, bool is_capture)
      : data(static_cast<size_t>(from) | (static_cast<size_t>(to) << 6) |
             (static_cast<size_t>(is_capture) << 14)) {}

  constexpr Move(Square from, Square to, Special special)
      : Move(from, to, special == Special::EN_PASSANT) {
    switch (special) {
    case Special::DOUBLE_PUSH:
    case Special::EN_PASSANT:
      data |= 1 << 12;
      break;
    case Special::KING_CASTLE:
      data |= 2 << 12;
      break;
    case Special::QUEEN_CASTLE:
      data |= 3 << 12;
      break;
    }
  }

  constexpr Move(Square from, Square to, bool is_capture,
                 Piece promoted_to_piece)
      : Move(from, to, is_capture) {
    data |= ((static_cast<size_t>(promoted_to_piece) - 1) << 12) | (1 << 15);
  }

  constexpr Square from() const { return static_cast<Square>(data & 0b111111); }

  constexpr Square to() const {
    return static_cast<Square>((data >> 6) & 0b111111);
  }

  constexpr bool is_capture() const { return data & (1 << 14); }

  constexpr bool is_promotion() const { return data & (1 << 15); }

  constexpr bool is_en_passant() const { return (data & flags) == 5 << 12; }

  constexpr bool is_castle() const {
    return (data & flags & ~(1 << 12)) == 2 << 12;
  }

  constexpr bool is_double_push() const { return (data & flags) == 1 << 12; }

  constexpr std::optional<Piece> promoted_to() const {
    if (is_promotion())
      return static_cast<Piece>(((data >> 12) & 0b11) + 1);

    return std::nullopt;
  }
};
