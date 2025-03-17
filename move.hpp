#include "types.hpp"
#include <cstdint>

enum Special : uint8_t {
  NOT_SPECIAL,
  DOUBLE_PUSH,
  KING_CASTLE,
  QUEEN_CASTLE,
  EN_PASSANT,
};

struct __attribute__((packed)) Move {
  // https://www.chessprogramming.org/Encoding_Moves#From-To_Based

  Square from : 6;
  Square to : 6;
  bool is_promotion : 1, is_capture : 1;
  uint8_t special : 2;

  constexpr Move() { *(uint16_t *)(this) = 0; }

  constexpr Move(Square from, Square to, bool is_promotion, bool is_capture,
                 uint8_t special = NOT_SPECIAL)
      : from(from), to(to), is_promotion(is_promotion), is_capture(is_capture),
        special(special) {
    if (special == EN_PASSANT)
      this->special = 1;
  }

  constexpr bool is_en_passant() const {
    return !is_promotion && is_capture && special == 1;
  }

  constexpr bool is_castle() const {
    return !is_promotion && !is_capture && (special & 2);
  }

  constexpr bool is_double_push() const {
    return !is_promotion && !is_capture && special == DOUBLE_PUSH;
  }
};
