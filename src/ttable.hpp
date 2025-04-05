#pragma once

#include "move.hpp"
#include <vector>

struct TTNode {
  enum class Type : uint8_t { EXACT, LOWERBOUND, UPPERBOUND };
  uint64_t hash;
  Move best_move;
  short value, depth;
  Type type;
};

template <std::size_t N> class TTable {
public:
  TTable() : table(N) {}

  constexpr void insert(uint64_t hash, Move best_move, short value, short depth,
                        TTNode::Type flag) {
    table[hash % N] = {hash, best_move, value, depth, flag};
  }

  constexpr std::optional<TTNode> lookup(uint64_t hash) const {
    TTNode node = table[hash % N];

    if (node.hash == hash)
      return node;

    return std::nullopt;
  }

private:
  std::vector<TTNode> table;
};
