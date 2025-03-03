#include "helper.hpp"
#include "side.hpp"
#include <cstring>
#include <print>

std::array<Bitboard, 64> init_knight_moves() {
  std::array<Bitboard, 64> knight_moves;

  for (uint8_t pos = 0; pos < 64; ++pos) {
    Bitboard bitboard = 1ULL << pos, moves = 0;

    // Up 2, Left 1
    moves |= (bitboard & ~(select_file(0) | select_rank(6) | select_rank(7)))
             << 15;

    // Up 2, Right 1
    moves |= (bitboard & ~(select_file(7) | select_rank(6) | select_rank(7)))
             << 17;

    // Down 2, Right 1
    moves |=
        (bitboard & ~(select_file(7) | select_rank(0) | select_rank(1))) >> 15;

    // Down 2, Left 1
    moves |=
        (bitboard & ~(select_file(0) | select_rank(0) | select_rank(1))) >> 17;

    // Left 2, Up 1
    moves |= (bitboard & ~(select_rank(7) | select_file(0) | select_file(1)))
             << 6;

    // Left 2, Down 1
    moves |=
        (bitboard & ~(select_rank(0) | select_file(0) | select_file(1))) >> 10;

    // Right 2, Down 1
    moves |=
        (bitboard & ~(select_rank(0) | select_file(6) | select_file(7))) >> 6;

    // Right 2, Up 1
    moves |= (bitboard & ~(select_rank(7) | select_file(6) | select_file(7)))
             << 10;

    knight_moves[pos] = moves;
  }

  return knight_moves;
}

std::array<Bitboard, 64> init_king_moves() {
  std::array<Bitboard, 64> king_moves;

  for (uint8_t pos = 0; pos < 64; ++pos) {
    Bitboard bitboard = 1ULL << pos, moves = 0;

    moves |= (bitboard & ~(select_rank(0) | select_file(0))) >> 9;
    moves |= (bitboard & ~(select_rank(0) | select_file(7))) >> 7;
    moves |= (bitboard & ~(select_rank(7) | select_file(0))) << 7;
    moves |= (bitboard & ~(select_rank(7) | select_file(7))) << 9;
    moves |= (bitboard & ~select_file(0)) >> 1;
    moves |= (bitboard & ~select_file(7)) << 1;
    moves |= (bitboard & ~select_rank(0)) >> 8;
    moves |= (bitboard & ~select_rank(7)) << 8;

    king_moves[pos] = moves;
  }

  return king_moves;
}
std::array<std::array<Bitboard, 64>, NUM_SIDES> init_pawn_moves() {
  std::array<std::array<Bitboard, 64>, NUM_SIDES> pawn_moves;
  memset(pawn_moves.data(), 0, sizeof(pawn_moves));

  for (uint8_t pos = 0; pos < 64; ++pos) {
    Bitboard bitboard = 1ULL << pos;
    std::array<Bitboard, NUM_SIDES> moves{0, 0};

    moves[WHITE] = ((bitboard & ~(select_rank(0) | select_rank(7))) << 8) |
                   ((bitboard & select_rank(1)) << 16);

    moves[BLACK] = ((bitboard & ~(select_rank(0) | select_rank(7))) >> 8) |
                   ((bitboard & select_rank(6)) >> 16);

    for (uint8_t side = 0; side < NUM_SIDES; ++side)
      pawn_moves[side][pos] = moves[side];
  }

  return pawn_moves;
}

std::array<std::array<Bitboard, 64>, NUM_SIDES> init_pawn_attacks() {
  std::array<std::array<Bitboard, 64>, NUM_SIDES> pawn_attacks;
  memset(pawn_attacks.data(), 0, sizeof(pawn_attacks));

  for (uint8_t pos = 0; pos < 64; ++pos) {
    Bitboard bitboard = 1ULL << pos;
    std::array<Bitboard, NUM_SIDES> moves{0, 0};

    moves[WHITE] =
        ((bitboard & ~(select_rank(0) | select_rank(7) | select_file(0)))
         << 7) |
        ((bitboard & ~(select_rank(0) | select_rank(7) | select_file(7))) << 9);

    moves[BLACK] =
        ((bitboard & ~(select_rank(0) | select_rank(7) | select_file(0))) >>
         9) |
        ((bitboard & ~(select_rank(0) | select_rank(7) | select_file(7))) >> 7);

    for (uint8_t side = 0; side < NUM_SIDES; ++side)
      pawn_attacks[side][pos] = moves[side];
  }

  return pawn_attacks;
}

int main() {
  auto x = init_pawn_moves();
  auto y = init_pawn_attacks();
  for (Bitboard bitboard : y[BLACK])
    std::print("{:#X}, ", bitboard);
}
