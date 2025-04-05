#include "board.hpp"
#include "eval.hpp"
#include "ttable.hpp"
#include <chrono>

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
      timed_out = std::chrono::steady_clock::now() >= deadline;

    return timed_out;
  }

  int qsearch(const Board &board, int ply, int alpha, int beta) {
    if (is_time_up())
      return 0;

    int stand_pat = Eval::eval(board), best_value = stand_pat;

    alpha = std::max(alpha, stand_pat);

    if (stand_pat >= beta)
      return stand_pat;

    uint64_t hash = board.hash();
    std::optional<TTNode> node = ttable.lookup(board.hash());

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

    if (board.is_draw())
      return 0;

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

    hashes.push_back(hash);
    Move best_move{};

    for (Move move : std::views::filter(moves, &Move::is_capture)) {
      Board copy = board;
      copy.make_move(move);

      if (copy.is_legal()) {
        int value = -qsearch(copy, ply + 1, -beta, -alpha);

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
    ++nodes_searched;

    ttable.insert(hash, best_move, best_value, 0,
                  best_value <= original_alpha ? TTNode::Type::UPPERBOUND
                  : best_value >= beta         ? TTNode::Type::LOWERBOUND
                                               : TTNode::Type::EXACT);

    return best_value;
  }

  int pvs(const Board &board, int depth, int ply = 0, int alpha = -INF,
          int beta = INF) {
    if (is_time_up())
      return 0;

    uint64_t hash = board.hash();
    std::optional<TTNode> node = ttable.lookup(board.hash());

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

    if (depth == 0)
      return qsearch(board, ply, alpha, beta);

    if (ply > 0 && (std::ranges::contains(hashes, hash) || board.is_draw()))
      return 0;

    bool first_move = true;
    int best_value = -INF, original_alpha = alpha;
    Move best_move{};

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

    hashes.push_back(hash);

    for (Move move : moves) {
      Board copy = board;
      copy.make_move(move);

      if (copy.is_legal()) {
        int value;

        if (first_move) {
          first_move = false;
          value = -pvs(copy, depth - 1, ply + 1, -beta, -alpha);
        } else {
          value = -pvs(copy, depth - 1, ply + 1, -(alpha + 1), -alpha);

          if (alpha < value && value < beta)
            value = -pvs(copy, depth - 1, ply + 1, -beta, -alpha);
        }

        if (timed_out) {
          hashes.pop_back();
          return 0;
        }

        if (value > best_value) {
          best_value = value;
          best_move = move;

          if (ply == 0) {
            best_global_move = move;
            best_global_value = value;
          }
        }

        alpha = std::max(alpha, value);

        if (alpha >= beta)
          break;
      }
    }

    hashes.pop_back();
    ++nodes_searched;

    if (best_value == -INF)
      best_value = board.is_check() ? ply - CHECKMATE : 0;

    int tt_value = best_value;

    if (best_value < -CHECKMATE_THRESHOLD)
      tt_value -= ply;
    else if (best_value > CHECKMATE_THRESHOLD)
      tt_value += ply;

    ttable.insert(hash, best_move, tt_value, depth,
                  best_value <= original_alpha ? TTNode::Type::UPPERBOUND
                  : best_value >= beta         ? TTNode::Type::LOWERBOUND
                                               : TTNode::Type::EXACT);

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

    for (int depth = 1; !timed_out; ++depth) {
      pvs(board, depth);

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
    }

    Board copy = board;
    copy.make_move(best_global_move);

    hashes.push_back(board.hash());
    hashes.push_back(copy.hash());

    return best_global_move;
  }
};
