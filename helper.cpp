#include "helper.hpp"
#include <print>

void print_bitboard(Bitboard bitboard) {
  std::puts("\t\tA B C D E F G H\n");

  for (int8_t rank = 7; rank >= 0; --rank) {
    std::print("\t{}\t", rank);

    for (uint8_t file = 0; file < 8; ++file)
      std::print("{} ",
                 static_cast<Bitboard>((bitboard & get_bit(rank, file)) != 0));

    std::println("\t{}", rank);
  }

  std::println("\n\t\tA B C D E F G H {:#X}\n", bitboard);
}
Bitboard select_file(uint8_t file) {
  Bitboard bitboard = 0;

  for (uint8_t rank = 0; rank < 8; ++rank)
    bitboard |= get_bit(rank, file);

  return bitboard;
}

Bitboard select_rank(uint8_t rank) {
  Bitboard bitboard = 0;

  for (uint8_t file = 0; file < 8; ++file)
    bitboard |= get_bit(rank, file);

  return bitboard;
}
