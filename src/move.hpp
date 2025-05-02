#pragma once

#include "piece.hpp"
#include "square.hpp"

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
      : data(from.raw() | (to.raw() << 6) | (is_capture << 14)) {}

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
    data |= ((promoted_to_piece.raw() - 1) << 12) | (1 << 15);
  }

  constexpr bool operator==(Move other) const { return data == other.data; }

  constexpr Square from() const { return Square(data & 0b111111); }

  constexpr Square to() const { return Square((data >> 6) & 0b111111); }

  constexpr bool is_capture() const { return data & (1 << 14); }

  constexpr bool is_promotion() const { return data & (1 << 15); }

  constexpr bool is_en_passant() const { return (data & flags) == 5 << 12; }

  constexpr bool is_castle() const {
    return !is_promotion() && (data & (2 << 12));
  }

  constexpr bool is_double_push() const { return (data & flags) == 1 << 12; }

  constexpr bool is_quiet() const { return !is_capture() && !is_promotion(); }

  constexpr Piece promoted_to() const {
    return Piece(static_cast<Piece::Literal>(((data >> 12) & 0b11) + 1));
  }

  constexpr uint16_t raw() const { return data; }

  constexpr std::string uci() {
    std::string ans = from().to_string() + to().to_string();

    if (is_promotion())
      ans += promoted_to().repr();

    return ans;
  }
};

struct ScoredMove : public Move {
  uint32_t score;

  ScoredMove() = default;

  template <typename... Args>
  ScoredMove(uint32_t score, Args &&...args)
      : Move(std::forward<Args>(args)...), score(score) {}

  constexpr auto operator<=>(const ScoredMove &other) const {
    return score <=> other.score;
  };
};

struct ViriMove {
  uint16_t data;

  constexpr ViriMove(Move move) {
    data = move.from().raw();

    if (move.is_en_passant())
      data |= 0x4000;
    else if (move.is_castle())
      data |= 0x8000;
    else if (move.is_promotion())
      data |= 0xC000 | ((move.promoted_to().raw() - 1) << 12);

    Square to = move.to();

    if (move.is_castle()) {
      if (move.to() == Squares::C1)
        to = Squares::A1;
      else if (move.to() == Squares::C8)
        to = Squares::A8;
      else if (move.to() == Squares::G1)
        to = Squares::H1;
      else if (move.to() == Squares::G8)
        to = Squares::H8;
    }

    data |= to.raw() << 6;
  }
};
