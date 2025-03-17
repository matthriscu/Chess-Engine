#include "bitboard.hpp"
#include "enum_array.hpp"
#include "magic.hpp"
#include "types.hpp"

template <Piece>
inline constexpr enum_array<Square, Bitboard, NUM_SQUARES> pseudoattacks{};

template <>
inline constexpr enum_array<Square, Bitboard, NUM_SQUARES>
    pseudoattacks<Piece::KNIGHT> = []() {
      enum_array<Square, Bitboard, NUM_SQUARES> moves;

      for (Square square : ALL_SQUARES) {
        Bitboard bb = get_bit(square);

        moves[square] = ((bb & ~(files[0] | ranks[6] | ranks[7])) << 15) |
                        ((bb & ~(files[7] | ranks[6] | ranks[7])) << 17) |
                        ((bb & ~(files[7] | ranks[0] | ranks[1])) >> 15) |
                        ((bb & ~(files[0] | ranks[0] | ranks[1])) >> 17) |
                        ((bb & ~(ranks[7] | files[0] | files[1])) << 6) |
                        ((bb & ~(ranks[0] | files[0] | files[1])) >> 10) |
                        ((bb & ~(ranks[0] | files[6] | files[7])) >> 6) |
                        ((bb & ~(ranks[7] | files[6] | files[7])) << 10);
      }

      return moves;
    }();

template <>
inline constexpr enum_array<Square, Bitboard, NUM_SQUARES>
    pseudoattacks<Piece::KING> = []() {
      enum_array<Square, Bitboard, NUM_SQUARES> moves;

      for (Square square : ALL_SQUARES) {
        Bitboard bb = get_bit(square);

        moves[square] = ((bb & ~(ranks[0] | files[0])) >> 9) |
                        ((bb & ~(ranks[0] | files[7])) >> 7) |
                        ((bb & ~(ranks[7] | files[0])) << 7) |
                        ((bb & ~(ranks[7] | files[7])) << 9) |
                        ((bb & ~files[0]) >> 1) | ((bb & ~files[7]) << 1) |
                        ((bb & ~ranks[0]) >> 8) | ((bb & ~ranks[7]) << 8);
      }

      return moves;
    }();

inline constexpr enum_array<Side, enum_array<Square, Bitboard, NUM_SQUARES>,
                            NUM_SIDES>
    pawn_moves = []() {
      enum_array<Side, enum_array<Square, Bitboard, NUM_SQUARES>, NUM_SIDES>
          moves = {};

      for (Square square : ALL_SQUARES) {
        Bitboard bb = get_bit(square);

        moves[Side::WHITE][square] =
            ((bb & ~(ranks[0] | ranks[7])) << 8) | ((bb & ranks[1]) << 16);

        moves[Side::BLACK][square] =
            ((bb & ~(ranks[0] | ranks[7])) >> 8) | ((bb & ranks[6]) >> 16);
      }

      return moves;
    }();

inline constexpr enum_array<Side, enum_array<Square, Bitboard, NUM_SQUARES>,
                            NUM_SIDES>
    pawn_attacks = []() {
      enum_array<Side, enum_array<Square, Bitboard, NUM_SQUARES>, NUM_SIDES>
          attacks = {};

      for (Square square : ALL_SQUARES) {
        Bitboard bb = get_bit(square);

        attacks[Side::WHITE][square] =
            ((bb & ~files[0]) << 7) | ((bb & ~files[7]) << 9);

        attacks[Side::BLACK][square] =
            ((bb & ~files[0]) >> 9) | ((bb & ~files[7]) >> 7);
      }

      return attacks;
    }();

template <Piece P>
constexpr Bitboard attacks_bb(Square square, Bitboard occupied);

template <>
constexpr Bitboard attacks_bb<Piece::KING>(Square square, Bitboard) {
  return pseudoattacks<Piece::KING>[square];
}

template <>
constexpr Bitboard attacks_bb<Piece::KNIGHT>(Square square, Bitboard) {
  return pseudoattacks<Piece::KNIGHT>[square];
}

template <>
constexpr Bitboard attacks_bb<Piece::BISHOP>(Square square, Bitboard occupied) {
  FancyHash m = magics<Piece::BISHOP>[static_cast<size_t>(square)];
  return m.attacks[((occupied | m.mask) * m.hash) >> (64 - 9)];
}

template <>
constexpr Bitboard attacks_bb<Piece::ROOK>(Square square, Bitboard occupied) {
  FancyHash m = magics<Piece::ROOK>[static_cast<size_t>(square)];
  return m.attacks[((occupied | m.mask) * m.hash) >> (64 - 12)];
}

template <>
constexpr Bitboard attacks_bb<Piece::QUEEN>(Square square, Bitboard occupied) {
  return attacks_bb<Piece::ROOK>(square, occupied) |
         attacks_bb<Piece::BISHOP>(square, occupied);
}

constexpr Bitboard attacks_bb(Piece p, Square square, Bitboard occupied = 0) {
  switch (p) {
  case Piece::KNIGHT:
    return attacks_bb<Piece::KNIGHT>(square, occupied);
  case Piece::KING:
    return attacks_bb<Piece::KING>(square, occupied);
  case Piece::BISHOP:
    return attacks_bb<Piece::BISHOP>(square, occupied);
  case Piece::ROOK:
    return attacks_bb<Piece::ROOK>(square, occupied);
  case Piece::QUEEN:
    return attacks_bb<Piece::QUEEN>(square, occupied);
  default:
    return 0;
  }
}
