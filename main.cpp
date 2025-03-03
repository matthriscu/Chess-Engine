#include "board.hpp"
#include "constants.hpp"
#include "helper.hpp"
#include <print>

int main() {
  Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  for (auto x : rank_masks)
    print_bitboard(x);
}
