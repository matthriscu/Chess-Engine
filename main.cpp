#include "board.hpp"
#include <print>

int main() {
  Board board("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w "
              "- - 0 10");

  print_bitboard(board.pieces[WHITE][PAWN]);

  // for (Move move : board.generate_moves())
  //   println("{}{} - 1", to_string(static_cast<Square>(move.from)),
  //           to_string(static_cast<Square>(move.to)));
}
