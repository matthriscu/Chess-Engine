#pragma once

#include <limits>

static constexpr int INF = std::numeric_limits<short>::max();
static constexpr int CHECKMATE = INF - 1;
static constexpr int MAX_PLY = 256;
static constexpr int CHECKMATE_THRESHOLD = CHECKMATE - MAX_PLY;

enum class Direction {
  NORTH = 8,
  EAST = 1,
  SOUTH = -NORTH,
  WEST = -EAST,

  NORTH_WEST = NORTH + WEST,
  NORTH_EAST = NORTH + EAST,
  SOUTH_WEST = SOUTH + WEST,
  SOUTH_EAST = SOUTH + EAST
};
