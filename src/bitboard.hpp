#pragma once

#include "square.hpp"

class Bitboard {
public:
  constexpr Bitboard() : Bitboard(0) {}

  constexpr Bitboard(Square square) : data(1ULL << square.raw()) {}

  constexpr Bitboard(int rank, int file) : Bitboard(Square(rank, file)) {}

  constexpr Bitboard(uint64_t data) : data(data) {}

  constexpr uint64_t raw() const { return data; }

  constexpr operator bool() const { return data; }

  constexpr operator Square() const { return Square(std::countr_zero(data)); }

  constexpr Bitboard operator<<(int shift) const { return data << shift; }

  constexpr Bitboard &operator<<=(int shift) {
    data <<= shift;
    return *this;
  }

  constexpr Bitboard operator>>(int shift) const { return data >> shift; }

  constexpr Bitboard &operator>>=(int shift) {
    data >>= shift;
    return *this;
  }

  constexpr Bitboard operator~() const { return ~data; }

  constexpr Bitboard operator&(Bitboard other) const {
    return data & other.data;
  }

  constexpr Bitboard &operator&=(Bitboard other) {
    data &= other.data;
    return *this;
  }

  constexpr Bitboard operator|(Bitboard other) const {
    return data | other.data;
  }

  constexpr Bitboard &operator|=(Bitboard other) {
    data |= other.data;
    return *this;
  }

  constexpr Bitboard operator^(Bitboard other) const {
    return data ^ other.data;
  }

  constexpr Bitboard &operator^=(Bitboard other) {
    data ^= other.data;
    return *this;
  }

  constexpr Bitboard lsb() const { return 1ULL << std::countr_zero(data); }

  constexpr Square pop_lsb() {
    Bitboard bit = lsb();
    *this &= ~bit;

    return Square(bit);
  }

  constexpr int popcount() const { return std::popcount(data); }

  template <Direction D> constexpr Bitboard shift() const;

  constexpr Bitboard shift(Direction d) const {
    switch (d) {
    case Direction::NORTH:
      return shift<Direction::NORTH>();
    case Direction::EAST:
      return shift<Direction::EAST>();
    case Direction::SOUTH:
      return shift<Direction::SOUTH>();
    case Direction::WEST:
      return shift<Direction::WEST>();
    case Direction::NORTH_WEST:
      return shift<Direction::NORTH_WEST>();
    case Direction::NORTH_EAST:
      return shift<Direction::NORTH_EAST>();
    case Direction::SOUTH_WEST:
      return shift<Direction::SOUTH_WEST>();
    case Direction::SOUTH_EAST:
      return shift<Direction::SOUTH_EAST>();
    default:
      return 0;
    }
  }

private:
  uint64_t data;
};

namespace Bitboards {
static constexpr Bitboard FileA = 0x0101010101010101ULL;
static constexpr Bitboard FileB = FileA << 1;
static constexpr Bitboard FileC = FileA << 2;
static constexpr Bitboard FileD = FileA << 3;
static constexpr Bitboard FileE = FileA << 4;
static constexpr Bitboard FileF = FileA << 5;
static constexpr Bitboard FileG = FileA << 6;
static constexpr Bitboard FileH = FileA << 7;

static constexpr Bitboard Rank1 = 0xFF;
static constexpr Bitboard Rank2 = Rank1 << 8;
static constexpr Bitboard Rank3 = Rank1 << 16;
static constexpr Bitboard Rank4 = Rank1 << 24;
static constexpr Bitboard Rank5 = Rank1 << 32;
static constexpr Bitboard Rank6 = Rank1 << 40;
static constexpr Bitboard Rank7 = Rank1 << 48;
static constexpr Bitboard Rank8 = Rank1 << 56;
}; // namespace Bitboards

template <Direction D> constexpr Bitboard Bitboard::shift() const {
  switch (D) {
  case Direction::NORTH:
    return data << 8;
  case Direction::SOUTH:
    return data >> 8;
  case Direction::WEST:
    return (*this & ~Bitboards::FileA) >> 1;
  case Direction::EAST:
    return (*this & ~Bitboards::FileH) << 1;
  case Direction::NORTH_WEST:
    return (*this & ~Bitboards::FileA) << 7;
  case Direction::NORTH_EAST:
    return (*this & ~Bitboards::FileH) << 9;
  case Direction::SOUTH_WEST:
    return (*this & ~Bitboards::FileA) >> 9;
  case Direction::SOUTH_EAST:
    return (*this & ~Bitboards::FileH) >> 7;
  }
}

template <> struct std::formatter<Bitboard> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(Bitboard bb, std::format_context &ctx) const {
    auto out = ctx.out();

    out = std::format_to(out, "\t\tA B C D E F G H\n");

    for (int rank = 7; rank >= 0; --rank) {
      out = std::format_to(out, "\t{}\t", rank + 1);
      for (int file = 0; file < 8; ++file) {
        out = std::format_to(out, "{:d} ", bool(bb & Bitboard(rank, file)));
      }

      out = std::format_to(out, "\t{}\n", rank + 1);
    }

    return std::format_to(out, "\n\t\tA B C D E F G H\t\t{:#X}\n", bb.raw());
  }
};
