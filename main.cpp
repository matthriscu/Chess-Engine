#include "board.hpp"
#include <print>

int main() {
  // Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  Board board("3b4/8/8/8/3R2N1/8/Q7/8 w KQkq - 0 1");
  std::println("{}", board);

  print_bitboard(
      attacks_bb<QUEEN>(get_square(1, 0), board.all_pieces[ALL_SIDES]));
}
