#include "board.hpp"
#include "eval.hpp"
#include "ttable.hpp"
#include <algorithm>
#include <chrono>
#include <functional>

class Searcher {
  TTable ttable;
  std::vector<uint64_t> hashes;

  long nodes_searched;
  bool timed_out;

  std::chrono::system_clock::time_point start, deadline;

  Move best_root_move;
  int best_root_value;

  bool is_time_up() {
    if (nodes_searched % 1024 == 0)
      timed_out = best_root_move != Move{} &&
                  std::chrono::system_clock::now() >= deadline;

    return timed_out;
  }

  constexpr MoveList sorted_moves(const Board &board, Move tt_move,
                                  bool qsearch = false) const {
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

    auto it = std::find(moves.begin(), moves.end(), tt_move);

    if (it != moves.end())
      std::rotate(moves.begin(), it, it + 1);

    if (qsearch)
      moves.resize(num_captures);

    return moves;
  }

  int qsearch(const Board &board, int ply, int alpha, int beta) {
    if (is_time_up())
      return 0;

    ++nodes_searched;

    int stand_pat = Eval::eval(board);

    if (stand_pat >= beta)
      return stand_pat;

    if (board.is_draw())
      return 0;

    alpha = std::max(alpha, stand_pat);

    std::optional<TTNode> node = ttable.lookup(board.zobrist);

    if (node.has_value() &&
        (node->type == TTNode::Type::EXACT ||
         (node->type == TTNode::Type::UPPERBOUND && node->value <= alpha) ||
         (node->type == TTNode::Type::LOWERBOUND && node->value >= beta))) {
      if (node->value < -CHECKMATE_THRESHOLD)
        return node->value + ply;

      if (node->value > CHECKMATE_THRESHOLD)
        return node->value - ply;

      return node->value;
    }

    Move best_move{};
    int best_value = stand_pat;
    TTNode::Type tt_type = TTNode::Type::UPPERBOUND;

    hashes.push_back(board.zobrist);

    for (Move move : sorted_moves(
             board, node.has_value() ? node->best_move : Move(), true)) {
      Board copy = board;
      copy.make_move(move);

      if (copy.is_legal()) {
        int value = -qsearch(copy, ply + 1, -beta, -alpha);

        if (timed_out) {
          hashes.pop_back();
          return 0;
        }

        best_value = std::max(best_value, value);

        if (value > alpha) {
          alpha = value;
          best_move = move;
          tt_type = TTNode::Type::EXACT;
        }

        if (value >= beta) {
          tt_type = TTNode::Type::LOWERBOUND;
          break;
        }
      }
    }

    hashes.pop_back();

    ttable.insert(board.zobrist, best_move, best_value, 0, tt_type);

    return best_value;
  }

  int negamax(const Board &board, int depth, int ply = 0, int alpha = -INF,
              int beta = INF) {
    if (is_time_up())
      return 0;

    ++nodes_searched;

    if (depth == 0)
      return qsearch(board, ply, alpha, beta);

    if (ply > 0 &&
        (std::ranges::contains(hashes, board.zobrist) || board.is_draw()))
      return 0;

    std::optional<TTNode> node = ttable.lookup(board.zobrist);

    if (ply > 0 && node.has_value() && node->depth >= depth &&
        (node->type == TTNode::Type::EXACT ||
         (node->type == TTNode::Type::UPPERBOUND && node->value <= alpha) ||
         (node->type == TTNode::Type::LOWERBOUND && node->value >= beta))) {
      if (node->value < -CHECKMATE_THRESHOLD)
        return node->value + ply;

      if (node->value > CHECKMATE_THRESHOLD)
        return node->value - ply;

      return node->value;
    }

    Move best_move{};
    int best_value = -INF;
    TTNode::Type tt_type = TTNode::Type::UPPERBOUND;
    bool first_move = true;

    hashes.push_back(board.zobrist);

    for (Move move :
         sorted_moves(board, node.has_value() ? node->best_move : Move())) {
      Board copy = board;
      copy.make_move(move);

      if (copy.is_legal()) {
        int value;

        if (first_move) {
          first_move = false;
          value = -negamax(copy, depth - 1, ply + 1, -beta, -alpha);
        } else {
          value = -negamax(copy, depth - 1, ply + 1, -alpha - 1, -alpha);

          if (alpha < value && value < beta)
            value = -negamax(copy, depth - 1, ply + 1, -beta, -alpha);
        }

        if (timed_out) {
          hashes.pop_back();
          return 0;
        }

        best_value = std::max(best_value, value);

        if (value > alpha) {
          alpha = value;
          best_move = move;
          tt_type = TTNode::Type::EXACT;
        }

        if (value >= beta) {
          tt_type = TTNode::Type::LOWERBOUND;
          break;
        }
      }
    }

    hashes.pop_back();

    if (best_value == -INF)
      best_value = board.is_check() ? ply - CHECKMATE : 0;

    if (ply == 0) {
      best_root_value = best_value;
      best_root_move = best_move;
    }

    int tt_value = best_value;

    if (best_value < -CHECKMATE_THRESHOLD)
      tt_value -= ply;
    else if (best_value > CHECKMATE_THRESHOLD)
      tt_value += ply;

    ttable.insert(board.zobrist, best_move, tt_value, depth, tt_type);

    return best_value;
  }

public:
  constexpr void stop() { timed_out = true; }

  constexpr void resize_ttable(std::size_t new_size) {
    ttable.resize(new_size);
  }

  constexpr void clear() {
    hashes.clear();
    ttable = {};
  }

  Move search(const Board &board,
              std::chrono::system_clock::duration duration) {
    start = std::chrono::system_clock::now();
    deadline = start + duration;

    nodes_searched = 0;
    timed_out = false;

    best_root_move = Move{};

    for (int depth = 1;; ++depth) {
      best_root_value = -INF;

      negamax(board, depth);

      if (timed_out)
        break;

      std::optional<int> moves_to_mate;

      if (best_root_value + CHECKMATE <= 100)
        moves_to_mate = -(CHECKMATE + best_root_value + 1) / 2;
      else if (CHECKMATE - best_root_value <= 100)
        moves_to_mate = (CHECKMATE - best_root_value + 1) / 2;

      auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now() - start)
                         .count();

      std::println("info depth {} nodes {} nps {} score {} time {} pv {}",
                   depth, nodes_searched,
                   static_cast<int>(1000 * nodes_searched / (time_ms + 1)),
                   moves_to_mate.has_value()
                       ? std::string("mate ") + std::to_string(*moves_to_mate)
                       : std::string("cp ") + std::to_string(best_root_value),
                   time_ms, best_root_move.uci());
    }

    Board copy = board;
    copy.make_move(best_root_move);

    hashes.push_back(board.zobrist);
    hashes.push_back(copy.zobrist);

    return best_root_move;
  }
};
