#include "board.hpp"
#include <chrono>

class Searcher {
  static constexpr int INF = 1e9;

  int nodes_searched = 0;
  bool timed_out = false;
  std::chrono::steady_clock::time_point deadline;
  Move best_move_iter;

  bool is_time_up() {
    if (++nodes_searched == 1024) {
      timed_out = std::chrono::steady_clock::now() >= deadline;
      nodes_searched = 0;
    }

    return timed_out;
  }

  constexpr int quiesce(const Board &board, int ply, int alpha, int beta) {
    if (is_time_up())
      return 0;

    if (board.is_check())
      return pvs(board, 1, ply, alpha, beta);

    int stand_pat = board.evaluation();

    if (stand_pat >= beta)
      return stand_pat;

    if (alpha < stand_pat)
      alpha = stand_pat;

    bool found_legal_move = false;
    int best_value = -INF;

    MoveList moves = board.pseudolegal_moves();
    std::ranges::sort(moves, std::bind_front(&Board::move_comparator, &board));

    for (Move move : std::views::filter(moves, &Move::is_capture)) {
      Board copy = board;
      copy.make_move(move);

      if (copy.is_legal()) {
        found_legal_move = true;

        int value = -quiesce(copy, ply + 1, -beta, -alpha);

        if (timed_out)
          return 0;

        if (value >= beta)
          return value;

        best_value = std::max(best_value, value);
        alpha = std::max(alpha, value);
      }
    }

    return found_legal_move ? best_value : 0;
  }

  constexpr int pvs(const Board &board, int depth, int ply = 0,
                    int alpha = -INF, int beta = INF) {
    if (is_time_up())
      return 0;

    if (depth == 0)
      return quiesce(board, ply, alpha, beta);

    MoveList moves = board.pseudolegal_moves();
    std::ranges::sort(moves, std::bind_front(&Board::move_comparator, &board));

    bool first_move = true, found_legal_move = false;
    int best_value = -INF;

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

        if (timed_out)
          return 0;

        if (value > best_value)
          best_value = value;

        if (value > alpha) {
          alpha = value;

          if (ply == 0)
            best_move_iter = move;
        }

        if (alpha >= beta)
          break;
      }
    }

    return found_legal_move ? best_value : board.is_check() ? ply - INF : 0;
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
