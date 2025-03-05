#include "types.hpp"
#include <cstdint>

enum Special : uint8_t { NOT_SPECIAL, PROMOTION, EN_PASSANT, CASTLING };

struct __attribute__((packed)) Move {
  uint8_t from : 6;
  uint8_t to : 6;
  uint8_t promoted_to_piece : 2;
  uint8_t special : 2; // 1 - promotion, 2 - en_passant, 3 - castling

  Move(Square from, Square to, Special special = NOT_SPECIAL)
      : from(from), to(to), special(special) {}

  Move(Square from, Square to, Piece promoted_to_piece)
      : from(from), to(to), promoted_to_piece(promoted_to_piece - 1),
        special(PROMOTION) {}
};
