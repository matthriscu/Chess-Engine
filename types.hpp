#pragma once

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
