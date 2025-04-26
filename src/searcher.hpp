#include "board.hpp"
#include "eval.hpp"
#include "move.hpp"
#include "ttable.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>

class Searcher {
  TTable ttable{};
  std::vector<uint64_t> hashes{};
  Squares::Array<Squares::Array<int>> history{};

  long nodes_searched = 0;
  bool timed_out = false;

  std::chrono::system_clock::time_point start, deadline;

  Move best_root_move;

  std::array<std::array<Move, 2>, MAX_PLY> killer_moves;

  bool is_time_up() {
    if (++nodes_searched % 1024 == 0)
      timed_out = best_root_move != Move{} &&
                  std::chrono::system_clock::now() >= deadline;

    return timed_out;
  }

  template <bool QSearch = false>
  constexpr MoveList sorted_moves(const Board &board, int ply,
                                  Move tt_move) const {
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

    if constexpr (QSearch)
      moves.resize(std::remove_if(moves.begin(), moves.end(),
                                  [](Move m) { return !m.is_capture(); }) -
                   moves.begin());

    std::array<ScoredMove, MAX_MOVES> scored_moves;

    std::transform(
        moves.begin(), moves.end(), scored_moves.begin(), [&](Move move) {
          uint32_t score;

          if (move == tt_move)
            score = std::numeric_limits<uint32_t>::max();
          else if (move.is_capture())
            score = 1'000'000'000 +
                    mvv_lva_lookup[board.square_to_piece[move.to()]]
                                  [board.square_to_piece[move.from()]];
          else if (std::ranges::contains(killer_moves[ply], move))
            score = 1'000'000'000;
          else
            score = history[move.from()][move.to()];

          return ScoredMove(score, move);
        });

    std::stable_sort(scored_moves.begin(), scored_moves.begin() + moves.size(),
                     std::greater<>{});

    std::transform(scored_moves.begin(), scored_moves.begin() + moves.size(),
                   moves.begin(),
                   [](ScoredMove scored_move) { return scored_move; });

    return moves;
  }

  int qsearch(const Board &board, int ply, int alpha, int beta) {
    if (is_time_up())
      return 0;

    int stand_pat = Eval::eval(board);

    if (stand_pat >= beta)
      return stand_pat;

    if (board.is_draw())
      return 0;

    alpha = std::max(alpha, stand_pat);

    std::optional<TTNode> node = ttable.lookup(board.zobrist, ply);

    if (node.has_value() &&
        (node->type == TTNode::Type::EXACT ||
         (node->type == TTNode::Type::UPPERBOUND && node->value <= alpha) ||
         (node->type == TTNode::Type::LOWERBOUND && node->value >= beta)))
      return node->value;

    Move best_move{};
    int best_value = stand_pat;
    TTNode::Type tt_type = TTNode::Type::UPPERBOUND;

    hashes.push_back(board.zobrist);

    for (Move move : sorted_moves<true>(
             board, ply, node.has_value() ? node->best_move : Move())) {
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

    ttable.insert(board.zobrist, best_move, best_value, 0, tt_type, ply);

    return best_value;
  }

  template <bool PV>
  int negamax(const Board &board, int depth, int ply = 0, int alpha = -INF,
              int beta = INF) {
    if (is_time_up())
      return 0;

    if (depth == 0)
      return qsearch(board, ply, alpha, beta);

    if (ply > 0 &&
        (std::ranges::contains(hashes, board.zobrist) || board.is_draw()))
      return 0;

    std::optional<TTNode> node = ttable.lookup(board.zobrist, ply);
    const bool is_check = board.is_check();
    int static_eval = Eval::eval(board);

    if constexpr (!PV) {
      if (node.has_value() && node->depth >= depth &&
          (node->type == TTNode::Type::EXACT ||
           (node->type == TTNode::Type::UPPERBOUND && node->value <= alpha) ||
           (node->type == TTNode::Type::LOWERBOUND && node->value >= beta)))
        return node->value;

      if (!is_check && static_eval < CHECKMATE_THRESHOLD &&
          static_eval >= beta + depth * 100)
        return static_eval;

      if (!is_check || (board.side_occupancy[board.stm] !=
                        (board.pieces[board.stm][Pieces::PAWN] |
                         board.pieces[board.stm][Pieces::KING]))) {
        Board copy = board;
        copy.make_null_move();

        int nmp_value = -negamax<false>(copy, std::max(depth - 3, 0), ply + 1,
                                        -beta, -(beta - 1));

        if (nmp_value >= beta)
          return nmp_value;
      }
    }

    Move best_move{};
    int best_value = -INF;
    TTNode::Type tt_type = TTNode::Type::UPPERBOUND;
    bool skip_quiets = false;

    hashes.push_back(board.zobrist);

    for (auto [i, move] : std::views::enumerate(sorted_moves(
             board, ply, node.has_value() ? node->best_move : Move()))) {
      Board copy = board;
      copy.make_move(move);

      int value;

      if (copy.is_legal()) {
        if (skip_quiets && move.is_quiet())
          continue;

        if (i > 0 && !PV && !is_check && best_value > -CHECKMATE_THRESHOLD &&
            depth <= 6 && static_eval + 200 + depth * 150 <= alpha &&
            move.is_quiet()) {
          skip_quiets = true;
          continue;
        }

        if (depth >= 2 && i > (ply == 0) && !is_check) {
          int reduction =
                  0.8 + 0.4 * std::log(depth) * std::log(std::max(i + 1, 1L)),
              reduced = depth - 1 - reduction;

          value = -negamax<false>(copy, reduced, ply + 1, -alpha - 1, -alpha);

          if (value > alpha && reduced < depth)
            value =
                -negamax<false>(copy, depth - 1, ply + 1, -alpha - 1, -alpha);
        } else if (!PV || i > 0) {
          value = -negamax<false>(copy, depth - 1, ply + 1, -alpha - 1, -alpha);
        }

        if (PV && (i == 0 || value > alpha))
          value = -negamax<true>(copy, depth - 1, ply + 1, -beta, -alpha);

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
          if (ply < MAX_PLY && move.is_quiet()) {
            if (move == killer_moves[ply][0])
              killer_moves[ply][1] = move;
            else
              killer_moves[ply][0] = move;
          }

          history[move.from()][move.to()] += depth * depth;
          tt_type = TTNode::Type::LOWERBOUND;
          break;
        }
      }
    }

    hashes.pop_back();

    if (best_value == -INF)
      best_value = board.is_check() ? ply - CHECKMATE : 0;

    if (ply == 0 && best_move != Move{})
      best_root_move = best_move;

    ttable.insert(board.zobrist, best_move, best_value, depth, tt_type, ply);

    return best_value;
  }

public:
  constexpr void stop() { timed_out = true; }

  constexpr void resize_ttable(std::size_t new_size) {
    ttable.resize(new_size);
  }

  constexpr void clear() {
    history = {};
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
    killer_moves = {};

    int best_root_value = -INF;

    for (int depth = 1;; ++depth) {
      int alpha = -INF, beta = INF, delta = 30;

      if (depth == 1) {
        alpha = -INF;
        beta = INF;
      } else {
        alpha = best_root_value - delta;
        beta = best_root_value + delta;
      }

      while (true) {
        int current = negamax<true>(board, depth, 0, alpha, beta);

        if (timed_out)
          break;

        if (current <= alpha)
          alpha -= delta;
        else if (current >= beta)
          beta += delta;
        else {
          best_root_value = current;
          break;
        }

        delta *= 2;
      }

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
