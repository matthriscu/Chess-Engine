#include "board.hpp"
#include <print>

int main() {
  Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  std::println("{}", board);
}
