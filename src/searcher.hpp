#include "board.hpp"
#include "eval.hpp"
#include "ttable.hpp"
#include <algorithm>
#include <chrono>
#include <functional>

class Searcher {
  std::vector<uint64_t> hashes;
  long nodes_searched;
  bool timed_out = false;
  std::chrono::steady_clock::time_point deadline;
  Move best_global_move;
  int best_global_value;
  TTable<1 << 22> ttable;

  bool is_time_up() {
    if (nodes_searched % 1024 == 0)
      timed_out = best_global_move != Move() &&
                  std::chrono::steady_clock::now() >= deadline;

    return timed_out;
  }

  int negamax(const Board &board, int depth, int ply = 0) {
    if (is_time_up())
      return 0;

    if (ply > 0 &&
        (std::ranges::contains(hashes, board.zobrist) || board.is_draw()))
      return 0;

    if (depth == 0)
      return Eval::eval(board);

    Move best_move = Move();
    int best_value = -INF;

    hashes.push_back(board.zobrist);

    for (Move move : board.pseudolegal_moves()) {
      Board copy = board;
      copy.make_move(move);

      if (copy.is_legal()) {
        int value = -negamax(copy, depth - 1, ply + 1);

        if (timed_out) {
          hashes.pop_back();
          return 0;
        }

        if (value > best_value) {
          best_value = value;
          best_move = move;
        }
      }
    }

    hashes.pop_back();
    ++nodes_searched;

    if (best_value == -INF)
      best_value = board.is_check() ? ply - CHECKMATE : 0;

    if (ply == 0 && best_value > best_global_value) {
      best_global_value = best_value;
      best_global_move = best_move;
    }

    return best_value;
  }

public:
  constexpr void clear() {
    hashes.clear();
    ttable = {};
  }

  Move search(const Board &board, int time) {
    auto start = std::chrono::steady_clock::now();
    nodes_searched = 0;
    timed_out = false;
    deadline = start + std::chrono::milliseconds(time);
    best_global_move = Move();
    best_global_value = -INF;

    for (int depth = 1;; ++depth) {
      negamax(board, depth);

      if (timed_out)
        break;

      auto time_ms = duration_cast<std::chrono::milliseconds>(
                         std::chrono::steady_clock::now() - start)
                         .count() +
                     1;

      std::optional<int> moves_to_mate;

      if (best_global_value + CHECKMATE <= 100)
        moves_to_mate = -(CHECKMATE + best_global_value + 1) / 2;
      else if (CHECKMATE - best_global_value <= 100)
        moves_to_mate = (CHECKMATE - best_global_value + 1) / 2;

      std::println("info depth {} nodes {} nps {} {} time {} pv {}", depth,
                   nodes_searched, 1000 * nodes_searched / time_ms,
                   moves_to_mate.has_value()
                       ? std::string("mate ") + std::to_string(*moves_to_mate)
                       : std::string("score cp ") +
                             std::to_string(best_global_value),
                   time_ms, best_global_move.uci());

      best_global_value = -INF;
    }

    Board copy = board;
    copy.make_move(best_global_move);

    hashes.push_back(board.hash());
    hashes.push_back(copy.hash());

    return best_global_move;
  }
};
