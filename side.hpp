#pragma once

#include "enumarray.hpp"

class Side {
public:
  enum class Literal { WHITE, BLACK };

  constexpr Side() : Side(Literal::WHITE) {}

  constexpr Side(Literal data) : data(data) {}

  constexpr operator Literal() const { return data; }

  constexpr Side operator~() const {
    return data == Literal::WHITE ? Literal::BLACK : Literal::WHITE;
  }

private:
  Literal data;
};

namespace Sides {
using enum Side::Literal;

inline constexpr int NUM = 2;

inline constexpr std::array<Side, NUM> ALL{WHITE, BLACK};

template <typename T> using Array = EnumArray<Side::Literal, T, NUM>;
} // namespace Sides
