#include "board.hpp"
#include <chrono>
#include <print>

void perft(Board board, size_t depth) {
  auto rec = [&](this auto self, Board board, size_t depth) {
    if (depth == 0) {
      return 1UZ;
    }

    size_t ans = 0;

    for (Move m : std::apply(std::views::take, board.generate_legal_moves())) {
      Board copy = board;
      copy.make_move(m);
      ans += self(copy, depth - 1);
    }

    return ans;
  };

  for (Move m : std::apply(std::views::take, board.generate_legal_moves())) {
    Board copy = board;

    copy.make_move(m);

    if (m.is_promotion)
      std::println("{}{}{} - {} ", to_string(m.from), to_string(m.to),
                   "nbrq"[m.special], rec(copy, depth - 1));
    else
      std::println("{}{} - {}", to_string(m.from), to_string(m.to),
                   rec(copy, depth - 1));
  }
}

int main() {
  auto time = std::chrono::high_resolution_clock::now();
  perft(Board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq "
              "- 0 1"),
        6);
  std::println("{}", std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::high_resolution_clock::now() - time)
                         .count());
}
