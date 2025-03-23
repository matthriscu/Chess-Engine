#include "board.hpp"
#include <chrono>

class Searcher {
  static constexpr int INF = 1e9;

  int nodes_searched = 0;
  bool timed_out = false;
  std::chrono::steady_clock::time_point deadline;
  Move best_move_iter;

  bool is_time_up() { return std::chrono::steady_clock::now() >= deadline; }

  constexpr int quiesce(const Board &board, int ply, int alpha, int beta) {
    if (++nodes_searched == 1024) {
      if (is_time_up()) {
        timed_out = true;
        return alpha;
      }

      nodes_searched = 0;
    }

    if (board.is_check())
      return pvs(board, 1, ply, alpha, beta);

    int stand_pat = board.evaluation();

    if (stand_pat >= beta)
      return stand_pat;

    if (alpha < stand_pat)
      alpha = stand_pat;

    bool found_legal_move = false;

    for (Move move :
         std::views::filter(board.pseudolegal_moves(), &Move::is_capture)) {
      Board copy = board;
      copy.make_move(move);

      if (copy.is_legal()) {
        found_legal_move = true;

        int value = -quiesce(copy, -beta, -alpha, ply + 1);

        if (value >= beta)
          return value;

        if (value > alpha)
          alpha = value;
      }
    }

    return found_legal_move ? alpha : 0;
  }

  constexpr int pvs(const Board &board, int depth, int ply = 0,
                    int alpha = -INF, int beta = INF) {
    if (++nodes_searched == 1024) {
      if (is_time_up()) {
        timed_out = true;
        return alpha;
      }

      nodes_searched = 0;
    }

    if (depth == 0)
      return board.evaluation();

    MoveList moves = board.pseudolegal_moves();
    std::ranges::sort(moves, std::bind_front(&Board::move_comparator, &board));

    bool first_move = true, found_legal_move = false;

    for (Move move : moves) {
      Board copy = board;
      copy.make_move(move);

      if (copy.is_legal()) {
        found_legal_move = true;

        int value;

        if (first_move) {
          value = -pvs(copy, depth - 1, ply + 1, -beta, -alpha);
          first_move = false;
        } else {
          value = -pvs(copy, depth - 1, ply + 1, -alpha - 1, -alpha);

          if (value > alpha && beta - alpha > 1)
            value = -pvs(copy, depth - 1, ply + 1, -beta, -alpha);
        }

        if (timed_out || value >= beta)
          return beta;

        if (value > alpha) {
          alpha = value;

          if (ply == 0)
            best_move_iter = move;
        }
      }
    }

    return found_legal_move ? alpha : board.is_check() ? ply - INF : 0;
  }

public:
  constexpr Move search(const Board &board, int time) {
    nodes_searched = 0;
    timed_out = false;
    deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(time);

    Move best_move;

    for (int depth = 1;; ++depth) {
      std::println("info depth {}", depth);
      best_move_iter = Move();

      pvs(board, depth);

      if (timed_out)
        break;

      best_move = best_move_iter;
    }

    return best_move;
  }
};
