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

  MoveList sorted_moves(const Board &board, bool qsearch = false) {
    static constexpr EnumArray<Piece::Literal, Pieces::Array<int>, 7>
        mvv_lva_lookup{
            // clang-format off
            15, 14, 13, 12, 11, 10,
            25, 24, 23, 22, 21, 20,
            35, 34, 33, 32, 31, 30,
            45, 44, 43, 42, 41, 40,
            55, 54, 53, 52, 51, 50,
            65, 64, 63, 62, 61, 60,
             0,  0,  0,  0,  0,  0,
            // clang-format on
        };

    MoveList moves = board.pseudolegal_moves();

    int num_captures = std::partition(moves.begin(), moves.end(),
                                      std::mem_fn(&Move::is_capture)) -
                       moves.begin();

    std::ranges::stable_sort(
        moves.begin(), moves.begin() + num_captures, std::greater<int>{},
        [&](Move m) {
          return mvv_lva_lookup[board.square_to_piece[m.to()]]
                               [board.square_to_piece[m.from()]];
        });

    if (qsearch)
      moves.resize(num_captures);

    return moves;
  }

  int negamax(const Board &board, int depth, int ply = 0, int alpha = -INF,
              int beta = INF) {
    if (is_time_up())
      return 0;

    ++nodes_searched;

    if (ply > 0 &&
        (std::ranges::contains(hashes, board.zobrist) || board.is_draw()))
      return 0;

    if (depth == 0)
      return Eval::eval(board);

    Move best_move = Move();
    int best_value = -INF;

    hashes.push_back(board.zobrist);

    for (Move move : sorted_moves(board)) {
      Board copy = board;
      copy.make_move(move);

      if (copy.is_legal()) {
        int value = -negamax(copy, depth - 1, ply + 1, -beta, -alpha);

        if (timed_out) {
          hashes.pop_back();
          return 0;
        }

        if (value > best_value) {
          best_value = value;
          best_move = move;
        }

        alpha = std::max(alpha, value);

        if (alpha >= beta)
          break;
      }
    }

    hashes.pop_back();

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

    hashes.push_back(board.zobrist);
    hashes.push_back(copy.zobrist);

    return best_global_move;
  }
};
