#pragma once

#include "move.hpp"

static constexpr std::size_t MAX_MOVES = 256;

struct MoveList : public std::array<Move, MAX_MOVES> {
  constexpr std::size_t size() const { return len; }

  constexpr const Move *end() const { return data() + len; }

  template <typename... Params> constexpr void add(Params &&...params) {
    new (&(*this)[len++]) Move(std::forward<Params>(params)...);
  }

private:
  std::size_t len = 0;
};
