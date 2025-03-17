#include "board.hpp"
#include <chrono>
#include <print>
#include <ranges>

void perft(std::string_view fen, size_t depth) {
  auto rec = [&](this auto self, Board &board, size_t depth) {
    if (depth == 0) {
      return 1UZ;
    }

    size_t ans = 0;

    for (Move m : std::apply(std::views::take, board.generate_moves())) {
      Board copy = board;
      copy.make_move(m);

      if (!copy.is_check())
        ans += self(copy, depth - 1);
    }

    return ans;
  };

  Board board(fen);

  for (Move m : std::apply(std::views::take, board.generate_moves())) {
    Board copy = board;
    copy.make_move(m);

    if (!copy.is_check()) {
      if (m.is_promotion())
        std::println("{}{}{} - {} ", to_string(m.from()), to_string(m.to()),
                     piece_to_char(m.promoted_to().value(), Side::WHITE),
                     rec(copy, depth - 1));
      else
        std::println("{}{} - {}", to_string(m.from()), to_string(m.to()),
                     rec(copy, depth - 1));
    }
  }
}

int main() {
  auto time = std::chrono::high_resolution_clock::now();

  perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        6);

  std::println("{}", std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::high_resolution_clock::now() - time)
                         .count());
}
