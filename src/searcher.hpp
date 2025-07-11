#pragma once

#include "eval.hpp"
#include "move.hpp"
#include "ttable.hpp"
#include "tunable_params.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>

class Searcher {
  TTable ttable{};
  std::vector<uint64_t> hashes{};
  Squares::Array<Squares::Array<int>> history{};

  int64_t nodes_searched, hard_node_limit;
  bool cancel_search;

  std::chrono::system_clock::time_point start, deadline;

  Move best_root_move;

  std::array<std::array<Move, 2>, MAX_PLY> killer_moves;

  bool check_hard_limit() {
    return cancel_search =
               (cancel_search || nodes_searched >= hard_node_limit ||
                (nodes_searched % TIME_CHECK_FREQUENCY == 0 &&
                 std::chrono::system_clock::now() >= deadline));
  }

  template <bool QSearch = false, typename BoardType>
    requires std::derived_from<BoardType, Board>
  constexpr MoveList sorted_moves(const BoardType &board, int ply,
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

    std::ranges::transform(moves, scored_moves.begin(), [&](Move move) {
      static constexpr int CAPTURE_BASE = 1'000'000'000,
                           KILLER_SCORE = CAPTURE_BASE;
      uint32_t score;

      if (move == tt_move)
        score = std::numeric_limits<uint32_t>::max();
      else if (move.is_capture())
        score = CAPTURE_BASE +
                mvv_lva_lookup[board.square_to_piece[move.to()]]
                              [board.square_to_piece[move.from()]];
      else if (std::ranges::contains(killer_moves[ply], move))
        score = KILLER_SCORE;
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

  template <bool PV, typename BoardType>
    requires std::derived_from<BoardType, Board>
  int qsearch(const BoardType &board, int ply, int alpha, int beta) {
    if (check_hard_limit())
      return 0;

    ++nodes_searched;

    int stand_pat = board.eval();

    if (stand_pat >= beta)
      return stand_pat;

    if (board.is_draw())
      return 0;

    alpha = std::max(alpha, stand_pat);

    std::optional<TTNode> node = ttable.lookup(board.zobrist, ply);

    if (!PV && node.has_value() &&
        (node->type == TTNode::Type::EXACT ||
         (node->type == TTNode::Type::UPPERBOUND && node->value <= alpha) ||
         (node->type == TTNode::Type::LOWERBOUND && node->value >= beta)))
      return node->value;

    Move best_move{};
    int best_value = stand_pat;
    TTNode::Type tt_type = TTNode::Type::UPPERBOUND;

    hashes.push_back(board.zobrist);

    for (Move move :
         sorted_moves<true>(board, ply, node ? node->best_move : Move{})) {
      BoardType copy = board;
      copy.make_move(move);

      if (copy.is_legal()) {
        int value = -qsearch<PV>(copy, ply + 1, -beta, -alpha);

        if (cancel_search) {
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

  template <bool PV, typename BoardType>
    requires std::derived_from<BoardType, Board>
  int negamax(const BoardType &board, int depth, int ply = 0, int alpha = -INF,
              int beta = INF) {
    if (check_hard_limit())
      return 0;

    if (depth == 0)
      return qsearch<PV>(board, ply, alpha, beta);

    ++nodes_searched;

    if (ply > 0 &&
        (std::ranges::contains(hashes, board.zobrist) || board.is_draw()))
      return 0;

    std::optional<TTNode> node = ttable.lookup(board.zobrist, ply);
    const bool is_check = board.is_check();
    int static_eval = board.eval();

    if constexpr (!PV) {
      if (node.has_value() && node->depth >= depth &&
          (node->type == TTNode::Type::EXACT ||
           (node->type == TTNode::Type::UPPERBOUND && node->value <= alpha) ||
           (node->type == TTNode::Type::LOWERBOUND && node->value >= beta)))
        return node->value;

      // RFP
      if (!is_check && static_eval < CHECKMATE_THRESHOLD &&
          static_eval >= beta + depth * RFP_SCALE)
        return static_eval;

      // NMP
      if (!is_check || (board.side_occupancy[board.stm] !=
                        (board.pieces[board.stm][Pieces::PAWN] |
                         board.pieces[board.stm][Pieces::KING]))) {
        BoardType copy = board;
        copy.make_null_move();

        int nmp_value =
            -negamax<false>(copy, std::max(depth - NMP_DEPTH_REDUCTION, 0),
                            ply + 1, -beta, -(beta - 1));

        if (nmp_value >= beta)
          return nmp_value;
      }
    }

    Move best_move{};
    int best_value = -INF;
    TTNode::Type tt_type = TTNode::Type::UPPERBOUND;

    hashes.push_back(board.zobrist);

    for (auto [i, move] : std::views::enumerate(
             sorted_moves(board, ply, node ? node->best_move : Move()))) {
      BoardType copy = board;
      copy.make_move(move);

      int value = -INF;

      if (copy.is_legal()) {
        // LMR
        if (depth >= LMR_MIN_DEPTH && i > (ply == 0) && !is_check) {
          int reduction = LMR_A + LMR_B * std::log(depth) *
                                      std::log(std::max(i + 1, 1L)),
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

        if (cancel_search) {
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
  constexpr void stop() { cancel_search = true; }

  constexpr void resize_ttable(std::size_t new_size) {
    ttable.resize(new_size);
  }

  constexpr void clear() {
    history = {};
    hashes.clear();
    ttable = {};
  }

  constexpr void clear_hashes() { hashes.clear(); }

  constexpr bool check_threefold(uint64_t hash) const {
    return std::ranges::count(hashes, hash) >= 3;
  }

  constexpr void add_hash(uint64_t hash) { hashes.push_back(hash); }

  template <bool INFO = true, typename BoardType>
    requires std::derived_from<BoardType, Board>
  constexpr std::pair<Move, int16_t>
  search(const BoardType &board,
         std::optional<std::chrono::system_clock::duration> duration_opt,
         std::optional<int64_t> soft_node_limit_opt,
         std::optional<int64_t> hard_node_limit_opt,
         std::optional<int64_t> max_depth_opt) {
    start = std::chrono::system_clock::now();
    deadline = start + duration_opt.value_or(std::chrono::years(1));
    hard_node_limit =
        hard_node_limit_opt.value_or(std::numeric_limits<int64_t>::max());

    int64_t soft_node_limit = soft_node_limit_opt.value_or(
                std::numeric_limits<int64_t>::max()),
            max_depth =
                max_depth_opt.value_or(std::numeric_limits<int64_t>::max());

    nodes_searched = 0;
    cancel_search = false;

    best_root_move = Move{};
    killer_moves = {};

    int best_root_value = -INF;

    for (int depth = 1; nodes_searched <= soft_node_limit && depth <= max_depth;
         ++depth) {
      int alpha = -INF, beta = INF, delta = ASP_DELTA;

      if (depth == 1) {
        alpha = -INF;
        beta = INF;
      } else {
        alpha = best_root_value - delta;
        beta = best_root_value + delta;
      }

      while (true) {
        int current = negamax<true>(board, depth, 0, alpha, beta);

        if (cancel_search)
          break;

        if (current <= alpha)
          alpha = std::max(alpha - delta, -INF);
        else if (current >= beta)
          beta = std::min(beta + delta, INF);
        else {
          best_root_value = current;
          break;
        }

        delta *= ASP_MULTIPLIER;
      }

      if (cancel_search)
        break;

      std::optional<int> moves_to_mate;

      if (best_root_value <= -CHECKMATE_THRESHOLD)
        moves_to_mate = -(CHECKMATE + best_root_value + 1) / 2;
      else if (best_root_value >= CHECKMATE_THRESHOLD)
        moves_to_mate = (CHECKMATE - best_root_value + 1) / 2;

      if (INFO) {
        auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now() - start)
                           .count();

        std::println(
            "info depth {} nodes {} nps {} hashfull {} score {} time {} pv {}",
            depth, nodes_searched,
            static_cast<int>(1000 * nodes_searched / (time_ms + 1)),
            ttable.hashfull(),
            moves_to_mate.has_value()
                ? std::string("mate ") + std::to_string(*moves_to_mate)
                : std::string("cp ") + std::to_string(best_root_value),
            time_ms, best_root_move.uci());
      }
    }

    return {best_root_move, best_root_value};
  }
};
