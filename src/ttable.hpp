#pragma once

#include "eval.hpp"
#include "move.hpp"
#include <vector>

struct TTNode {
  enum class Type : uint8_t { EXACT, LOWERBOUND, UPPERBOUND };
  uint64_t hash;
  Move best_move;
  short value, depth;
  Type type;
};

class TTable {
public:
  constexpr TTable() : size(1 << 22), table(size) {}

  constexpr void resize(std::size_t new_size) {
    size = new_size;
    table.resize(new_size);
  }

  constexpr void insert(uint64_t hash, Move best_move, short value, short depth,
                        TTNode::Type flag, int ply) {
    if (value < -CHECKMATE_THRESHOLD)
      value -= ply;
    else if (value > CHECKMATE_THRESHOLD)
      value += ply;

    table[hash % size] = {hash, best_move, value, depth, flag};
  }

  constexpr std::optional<TTNode> lookup(uint64_t hash, int ply) {
    TTNode node = table[hash % size];

    if (node.hash == hash) {
      if (node.value < -CHECKMATE_THRESHOLD)
        node.value += ply;
      else if (node.value > CHECKMATE_THRESHOLD)
        node.value -= ply;

      return node;
    }

    return std::nullopt;
  }

  constexpr int hashfull() const {
    return std::count_if(table.begin(), table.begin() + 1000,
                         [](const TTNode &node) { return node.hash != 0; });
  }

private:
  std::size_t size;
  std::vector<TTNode> table;
};
