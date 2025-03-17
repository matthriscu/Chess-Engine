#include "bitboard.hpp"
#include <print>

void print_bitboard(Bitboard bitboard) {
  std::puts("\t\tA B C D E F G H\n");

  for (int8_t rank = 7; rank >= 0; --rank) {
    std::print("\t{}\t", rank + 1);

    for (uint8_t file = 0; file < 8; ++file)
      std::print("{:d} ", (bitboard & get_bit(rank, file)) != 0);

    std::println("\t{}", rank + 1);
  }

  std::println("\n\t\tA B C D E F G H {:#X}\n", bitboard);
}
