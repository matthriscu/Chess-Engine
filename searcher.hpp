#include "board.hpp"
#include <chrono>

class Searcher {
  static constexpr int INF = 1e9;

  int nodes_searched = 0;
  bool timed_out = false;
  std::chrono::steady_clock::time_point deadline;
  Move best_move_iter;
  int best_value_iter;

  bool is_time_up() { return std::chrono::steady_clock::now() >= deadline; }

  constexpr int negamax(const Board &board, int depth, int ply = 0,
                        int alpha = -INF, int beta = INF) {
    if (depth == 0)
      return board.evaluation();

    MoveList moves = board.pseudolegal_moves();
    std::ranges::sort(moves, std::bind_front(&Board::move_comparator, &board));

    for (Move move : moves) {
      if (++nodes_searched == 1024) {
        if (is_time_up()) {
          timed_out = true;
          break;
        }

        nodes_searched = 0;
      }

      Board copy = board;
      copy.make_move(move);

      if (copy.is_legal()) {
        alpha =
            std::max(alpha, -negamax(copy, depth - 1, ply + 1, -beta, -alpha));

        if (timed_out || alpha >= beta)
          break;

        if (ply == 0 && alpha > best_value_iter) {
          best_value_iter = alpha;
          best_move_iter = move;
        }
      }
    }

    return alpha;
  }

public:
  constexpr Move search(const Board &board, int time) {
    nodes_searched = 0;
    timed_out = false;
    deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(time);

    Move best_move;

    for (int depth = 1; !is_time_up(); ++depth) {
      std::println("info depth {}", depth);
      best_value_iter = -INF;
      best_move_iter = Move();

      negamax(board, depth);

      if (timed_out)
        break;

      best_move = best_move_iter;
    }

    return best_move;
  }
};
