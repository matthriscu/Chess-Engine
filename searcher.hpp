#include "board.hpp"
#include "ttable.hpp"
#include <chrono>

class Searcher {
  static constexpr int INF = 1e9;

  std::vector<uint64_t> hashes;
  int nodes_searched = 0;
  bool timed_out = false;
  std::chrono::steady_clock::time_point deadline;
  Move best_move_iter;
  TTable<1 << 22> ttable;

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

    alpha = std::max(alpha, stand_pat);

    if (stand_pat >= beta)
      return stand_pat;

    uint64_t hash = board.hash();

    if (std::ranges::count(hashes, hash) == 2 || board.is_draw())
      return 0;

    hashes.push_back(hash);

    for (Move move : std::views::filter(board.sorted_pseudolegal_moves(),
                                        &Move::is_capture)) {
      Board copy = board;
      copy.make_move(move);

      if (copy.is_legal()) {
        int value = -qsearch(copy, ply + 1, -beta, -alpha);

        if (timed_out)
          break;

        best_value = std::max(best_value, value);
        alpha = std::max(alpha, value);

        if (alpha >= beta)
          break;
      }
    }

    hashes.pop_back();

    return best_value;
  }

  int pvs(const Board &board, int depth, int ply = 0, int alpha = -INF,
          int beta = INF) {
    if (is_time_up())
      return alpha;

    std::optional<TTNode> node = ttable.lookup(board.hash());

    if (ply > 0 && node.has_value() && node->depth >= depth &&
        (node->type == TTNode::Type::EXACT ||
         (node->type == TTNode::Type::UPPERBOUND && node->value <= alpha) ||
         (node->type == TTNode::Type::LOWERBOUND && node->value >= beta)))
      return node->value;

    if (depth == 0)
      return qsearch(board, ply, alpha, beta);

    uint64_t hash = board.hash();

    if (std::ranges::count(hashes, hash) == 2 || board.is_draw())
      return 0;

    hashes.push_back(hash);

    bool first_move = true, found_legal_move = false;
    int best_value = -INF;
    Move best_move{};
    int original_alpha = alpha;

    MoveList moves = board.pseudolegal_moves();

    Move ttMove = node.and_then([](const TTNode &ttNode) {
                        return std::optional{ttNode.best_move};
                      })
                      .value_or(Move());

    std::ranges::stable_sort(moves, [&](Move a, Move b) {
      if (a == ttMove)
        return true;

      if (b == ttMove)
        return false;

      return board.move_comparator(a, b);
    });

    for (Move move : moves) {
      Board copy = board;
      copy.make_move(move);

      if (copy.is_legal()) {
        found_legal_move = true;

        int value;

        if (first_move) {
          first_move = false;
          value = -pvs(copy, depth - 1, ply + 1, -beta, -alpha);
        } else {
          value = -pvs(copy, depth - 1, ply + 1, -(alpha + 1), -alpha);

          if (alpha < value && value < beta)
            value = -pvs(copy, depth - 1, ply + 1, -beta, -alpha);
        }

        if (timed_out)
          break;

        if (value > best_value) {
          best_value = value;
          best_move = move;

          if (ply == 0)
            best_move_iter = move;
        }

        alpha = std::max(alpha, value);

        if (alpha >= beta)
          break;
      }
    }

    hashes.pop_back();

    if (!timed_out)
      ttable.insert(hash, best_move, best_value, depth,
                    best_value <= original_alpha ? TTNode::Type::UPPERBOUND
                    : best_value >= beta         ? TTNode::Type::LOWERBOUND
                                                 : TTNode::Type::EXACT);

    if (!found_legal_move)
      return board.is_check() ? ply - INF : 0;

    return best_value;
  }

public:
  constexpr void clear() {
    hashes.clear();
    ttable = {};
  }

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
