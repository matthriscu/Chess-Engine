#include "board.hpp"

void perft(std::string_view fen, size_t depth) {
  auto rec = [&](this auto self, Board &board, size_t depth) {
    if (depth == 0) {
      return 1UZ;
    }

    size_t ans = 0;

    for (Move m :
         std::apply(std::views::take, board.generate_pseudolegal_moves())) {
      Board copy = board;
      copy.make_move(m);

      if (!copy.is_check())
        ans += self(copy, depth - 1);
    }

    return ans;
  };

  Board board(fen);

  for (Move m :
       std::apply(std::views::take, board.generate_pseudolegal_moves())) {
    Board copy = board;
    copy.make_move(m);

    if (!copy.is_check()) {
      if (m.is_promotion())
        std::println("{}{}{} - {} ", to_string(m.from()), to_string(m.to()),
                     piece_to_char(m.promoted_to(), Side::WHITE),
                     rec(copy, depth - 1));
      else
        std::println("{}{} - {}", to_string(m.from()), to_string(m.to()),
                     rec(copy, depth - 1));
    }
  }
}
