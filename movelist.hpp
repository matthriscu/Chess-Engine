#pragma once

#include "move.hpp"
#include <cstddef>

static constexpr std::size_t MAX_MOVES = 256;

struct MoveList : public std::array<Move, MAX_MOVES> {
  constexpr std::size_t size() const { return len; }

  constexpr const Move *end() const { return data() + len; }

  constexpr void push_back(Move move) { (*this)[len++] = move; }

private:
  std::size_t len = 0;
};
