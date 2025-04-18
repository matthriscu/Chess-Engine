#pragma once

#include "board.hpp"

constexpr int perft(const Board &board, int depth) {
  if (depth == 0)
    return 1;

  int ans = 0;

  for (Move m : board.pseudolegal_moves()) {
    Board copy = board;
    copy.make_move(m);

    if (copy.is_legal())
      ans += perft(copy, depth - 1);
  }

  return ans;
}

void splitperft(const Board &board, int depth);
