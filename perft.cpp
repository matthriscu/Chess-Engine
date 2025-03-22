#include "board.hpp"

void perft(std::string_view fen, size_t depth) {
  auto rec = [&](this auto self, const Board &board, size_t depth) {
    if (depth == 0) {
      return 1UZ;
    }

    size_t ans = 0;

    for (Move m : board.pseudolegal_moves()) {
      Board copy = board;
      copy.make_move(m);

      if (copy.is_legal())
        ans += self(copy, depth - 1);
    }

    return ans;
  };

  Board board(fen);

  for (Move m : board.pseudolegal_moves()) {
    Board copy = board;
    copy.make_move(m);

    if (copy.is_legal()) {
      if (m.is_promotion())
        std::println("{}{}{} - {} ", m.from(), m.to(),
                     piece_to_char(m.promoted_to(), Side::WHITE),
                     rec(copy, depth - 1));
      else
        std::println("{}{} - {}", m.from(), m.to(), rec(copy, depth - 1));
    }
  }
}
