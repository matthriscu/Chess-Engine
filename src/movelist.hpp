#pragma once

#include "move.hpp"

static constexpr std::size_t MAX_MOVES = 256;

struct MoveList : public std::array<Move, MAX_MOVES> {
  constexpr std::size_t size() const { return len; }

  constexpr Move *end() { return begin() + len; }

  constexpr const Move *end() const { return begin() + len; }

  template <typename... Params> constexpr void add(Params &&...params) {
    new (&(*this)[len++]) Move(std::forward<Params>(params)...);
  }

  constexpr Move get_matching_move(Square from, Square to,
                                   Piece promoted_to) const {
    return *std::ranges::find_if(*this, [&](Move move) {
      return (move.from() == from && move.to() == to &&
              (!move.is_promotion() || move.promoted_to() == promoted_to));
    });
  }

  constexpr void resize(std::size_t n) { len = n; }

private:
  std::size_t len = 0;
};
