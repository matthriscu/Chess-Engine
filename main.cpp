#include "board.hpp"
#include "helper.hpp"
#include "magic.hpp"
#include <print>

int main() {
  // Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  Board board("3b4/8/8/8/3R2N1/8/Q7/8 w KQkq - 0 1");
  std::println("{}", board);

  print_bitboard(
      get_queen_attacks(get_square_index(1, 0), board.all_pieces[ALL_SIDES]));
}
