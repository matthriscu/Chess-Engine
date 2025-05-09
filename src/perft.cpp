#include "perft.hpp"

void splitperft(const Board &board, int depth) {
  int64_t total = 0;

  for (Move m : board.pseudolegal_moves()) {
    Board copy = board;
    copy.make_move(m);

    if (copy.is_legal()) {
      int64_t current = perft(copy, depth - 1);

      std::println("{} - {}", m.uci(), current);
      total += current;
    }
  }

  std::println("Total: {}", total);
}
