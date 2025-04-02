#include "board.hpp"
#include <chrono>
#include <functional>

class Searcher {
  static constexpr int INF = 1e9;

  std::vector<uint64_t> hashes;
  int nodes_searched = 0;
  bool timed_out = false;
  std::chrono::steady_clock::time_point deadline;
  Move best_move_iter;

  bool is_time_up() {
    if (++nodes_searched == 1024) {
      timed_out = std::chrono::steady_clock::now() >= deadline &&
                  best_move_iter != Move();
      nodes_searched = 0;
    }

    return timed_out;
  }

  int qsearch(const Board &board, int ply, int alpha, int beta) {
    if (is_time_up())
      return alpha;

    if (board.is_check())
      return pvs(board, 1, ply, alpha, beta);

    int stand_pat = board.evaluation(), best_value = stand_pat;

    if (stand_pat >= beta)
      return stand_pat;

    uint64_t hash = board.hash();

    if (std::ranges::count(hashes, hash) == 2 || board.is_draw())
      return 0;

    hashes.push_back(hash);

    alpha = std::max(alpha, stand_pat);

    MoveList moves = board.pseudolegal_moves();
    std::ranges::sort(moves, std::bind_front(&Board::move_comparator, &board));

    for (Move move : std::views::filter(moves, &Move::is_capture)) {
      Board copy = board;
      copy.make_move(move);

      if (copy.is_legal()) {

        int value = -qsearch(copy, ply + 1, -beta, -alpha);

        if (timed_out) {
          hashes.pop_back();
          return alpha;
        }

        if (value >= beta) {
          hashes.pop_back();
          return value;
        }

        best_value = std::max(best_value, value);
        alpha = std::max(alpha, value);
      }
    }

    hashes.pop_back();

    return best_value;
  }

  int pvs(const Board &board, int depth, int ply = 0, int alpha = -INF,
          int beta = INF) {

    if (is_time_up())
      return alpha;

    if (depth == 0)
      return qsearch(board, ply, alpha, beta);

    uint64_t hash = board.hash();

    if (std::ranges::count(hashes, hash) == 2 || board.is_draw())
      return 0;

    hashes.push_back(hash);

    MoveList moves = board.pseudolegal_moves();
    std::ranges::sort(moves, std::bind_front(&Board::move_comparator, &board));

    bool first_move = true, found_legal_move = false;
    int best_value = -INF;

    for (Move move : moves) {
      Board copy = board;
      copy.make_move(move);

      if (copy.is_legal()) {
        found_legal_move = true;
        first_move = false;

        int value;

        if (first_move)
          value = -pvs(copy, depth - 1, ply + 1, -beta, -alpha);
        else {
          value = -pvs(copy, depth - 1, ply + 1, -(alpha + 1), -alpha);

          if (alpha < value && value < beta)
            value = -pvs(copy, depth - 1, ply + 1, -beta, -alpha);
        }

        if (timed_out) {
          hashes.pop_back();
          return alpha;
        }

        if (value > best_value) {
          best_value = value;

          if (ply == 0)
            best_move_iter = move;
        }

        alpha = std::max(alpha, value);

        if (alpha >= beta)
          break;
      }
    }

    hashes.pop_back();

    if (!found_legal_move)
      return board.is_check() ? ply - INF : 0;

    return best_value;
  }

public:
  constexpr void clear() { hashes.clear(); }

  Move search(const Board &board, int time) {
    nodes_searched = 0;
    timed_out = false;
    deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(time);
    best_move_iter = Move();

    Move best_move;

    for (int depth = 1;; ++depth) {
      std::println("info depth {}", depth);

      pvs(board, depth);

      if (depth != 1 && timed_out)
        break;

      best_move = best_move_iter;
    }

    Board copy = board;
    copy.make_move(best_move);

    hashes.push_back(board.hash());
    hashes.push_back(copy.hash());

    return best_move;
  }
};
