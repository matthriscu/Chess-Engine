#pragma once

#include "attacks.hpp"
#include "bitboard.hpp"
#include "common.hpp"
#include "enumarray.hpp"
#include "move.hpp"
#include "movelist.hpp"
#include "types.hpp"
#include <chrono>
#include <cstdint>
#include <functional>
#include <print>
#include <ranges>
#include <vector>

struct Board {
  EnumArray<Side, EnumArray<Piece, Bitboard, NUM_PIECES>, NUM_SIDES> pieces;
  EnumArray<Side, Bitboard, NUM_SIDES + 1> side_occupancy;
  EnumArray<Square, std::optional<Piece>, NUM_SQUARES> square_to_piece;
  Bitboard general_occupancy;

  int halfmove_clock;
  std::optional<Square> en_passant_square;
  Side stm;
  EnumArray<Side, std::array<bool, 2>, NUM_SIDES> castling_rights;

  constexpr Board() = default;

  constexpr Board(std::string_view fen_string) {
    std::vector<std::string_view> tokens = string_tokenizer(fen_string);

    pieces = {};
    side_occupancy = {};
    square_to_piece = {};

    int rank = 7, file = 0;

    for (char c : tokens[0])
      if (c == '/') {
        --rank;
        file = 0;
      } else if (isdigit(c))
        file += c - '0';
      else {
        const Piece piece = char_to_piece(c).value();
        const Side side = 'A' <= c && c <= 'Z' ? Side::WHITE : Side::BLACK;

        pieces[side][piece] |= Bitboard(rank, file);

        side_occupancy[side] |= Bitboard(rank, file);
        square_to_piece[get_square(rank, file)] = piece;

        ++file;
      }

    general_occupancy =
        side_occupancy[Side::WHITE] | side_occupancy[Side::BLACK];
    stm = tokens[1] == "w" ? Side::WHITE : Side::BLACK;
    castling_rights[Side::WHITE][0] = tokens[2].contains('K');
    castling_rights[Side::WHITE][1] = tokens[2].contains('Q');
    castling_rights[Side::BLACK][0] = tokens[2].contains('k');
    castling_rights[Side::BLACK][1] = tokens[2].contains('q');
    en_passant_square = from_string(tokens[3]);
    std::from_chars(tokens[4].begin(), tokens[4].end(), halfmove_clock);
  }

  constexpr void make_move(Move m) {
    auto add_piece = [&](Side side, Piece piece, Square square) {
      pieces[side][piece] |= Bitboard(square);
      square_to_piece[square] = piece;
    };

    auto remove_piece = [&](Side side, Piece piece, Square square) {
      pieces[side][piece] &= ~Bitboard(square);
      square_to_piece[square] = std::nullopt;
    };

    auto move_piece = [&](Side side, Piece piece, Square from, Square to) {
      pieces[side][piece] ^= Bitboard(from) | Bitboard(to);
      square_to_piece[from] = std::nullopt;
      square_to_piece[to] = piece;
    };

    Piece moved_piece = square_to_piece[m.from()].value();

    if (m.is_en_passant())
      remove_piece(opponent(stm), Piece::PAWN,
                   stm == Side::WHITE ? south(en_passant_square.value())
                                      : north(en_passant_square.value()));
    else if (m.is_capture())
      remove_piece(opponent(stm), square_to_piece[m.to()].value(), m.to());

    move_piece(stm, moved_piece, m.from(), m.to());

    if (moved_piece == Piece::KING)
      castling_rights[stm] = {};

    if (m.from() == Square::h1 || m.to() == Square::h1)
      castling_rights[Side::WHITE][0] = false;
    else if (m.from() == Square::a1 || m.to() == Square::a1)
      castling_rights[Side::WHITE][1] = false;

    if (m.from() == Square::h8 || m.to() == Square::h8)
      castling_rights[Side::BLACK][0] = false;
    else if (m.from() == Square::a8 || m.to() == Square::a8)
      castling_rights[Side::BLACK][1] = false;

    if (m.is_promotion()) {
      remove_piece(stm, Piece::PAWN, m.to());
      add_piece(stm, m.promoted_to(), m.to());
    }

    if (m.is_castle()) {
      if (m.to() == Square::g1)
        move_piece(stm, Piece::ROOK, Square::h1, Square::f1);
      else if (m.to() == Square::c1)
        move_piece(stm, Piece::ROOK, Square::a1, Square::d1);
      else if (m.to() == Square::g8)
        move_piece(stm, Piece::ROOK, Square::h8, Square::f8);
      else if (m.to() == Square::c8)
        move_piece(stm, Piece::ROOK, Square::a8, Square::d8);
    }

    for (Side side : {Side::WHITE, Side::BLACK}) {
      side_occupancy[side] = 0;

      for (Piece piece : {Piece::PAWN, Piece::KNIGHT, Piece::BISHOP,
                          Piece::ROOK, Piece::QUEEN, Piece::KING})
        side_occupancy[side] |= pieces[side][piece];
    }

    if (m.is_double_push())
      en_passant_square = std::make_optional(
          stm == Side::WHITE ? south(m.to()) : north(m.to()));
    else
      en_passant_square = std::nullopt;

    general_occupancy =
        side_occupancy[Side::WHITE] | side_occupancy[Side::BLACK];
    stm = opponent(stm);
    halfmove_clock =
        moved_piece == Piece::PAWN || m.is_capture() ? 0 : halfmove_clock + 1;
  }

  constexpr Bitboard threats(Side side) const {
    Bitboard threats = 0;

    for (Piece p : {Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN,
                    Piece::KING}) {
      Bitboard bb = pieces[side][p];

      while (bb)
        threats |= attacks_bb(p, bb.pop_lsb(), general_occupancy);
    }

    Bitboard bb = pieces[side][Piece::PAWN];
    while (bb)
      threats |= pawn_attacks[side][bb.pop_lsb()];

    return threats & ~side_occupancy[side];
  }

  constexpr bool is_attacked(Square square, Side side) const {
    for (Piece p :
         {Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN, Piece::KING})
      if (attacks_bb(p, square, general_occupancy) & pieces[side][p])
        return true;

    return pawn_attacks[opponent(side)][square] & pieces[side][Piece::PAWN];
  }

  constexpr bool is_legal() const {
    return !is_attacked(
        static_cast<Square>(Square(pieces[opponent(stm)][Piece::KING])), stm);
  }

  constexpr bool is_check() const {
    return is_attacked(Square(pieces[stm][Piece::KING]), opponent(stm));
  }

  constexpr void generate_regular_moves(MoveList &move_list) const {
    for (Piece p : {Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN,
                    Piece::KING}) {
      Bitboard bb = pieces[stm][p];

      while (bb) {
        Square from = bb.pop_lsb();

        Bitboard attacks =
            attacks_bb(p, from, general_occupancy) & ~side_occupancy[stm];

        while (attacks) {
          Square to = attacks.pop_lsb();
          move_list.push_back(Move(from, to, square_to_piece[to].has_value()));
        }
      }
    }
  }

  constexpr void generate_castling_moves(MoveList &move_list) const {
    static constexpr EnumArray<Side, std::array<Bitboard, 2>, NUM_SIDES>
        castling_free_masks = {0x60, 0xE, 0x6000000000000000,
                               0xE00000000000000};

    static constexpr EnumArray<Side, std::array<Bitboard, 2>, NUM_SIDES>
        castling_attack_masks = {0x70, 0x1C, 0x7000000000000000,
                                 0x1c00000000000000};

    Bitboard threats_bb = threats(opponent(stm));
    size_t castling_rank = stm == Side::WHITE ? 0 : 7;
    Square king_square = get_square(castling_rank, 4);

    for (size_t i = 0; i < 2; ++i)
      if (castling_rights[stm][i] &&
          !(general_occupancy & castling_free_masks[stm][i]) &&
          !(threats_bb & castling_attack_masks[stm][i])) {
        move_list.push_back(
            Move(king_square, get_square(castling_rank, 6 - 4 * i),
                 i == 0 ? Special::KING_CASTLE : Special::QUEEN_CASTLE));
      }
  }

  constexpr void generate_pawn_moves(MoveList &move_list) const {
    // Function pointers to disambiguate templated functions
    Direction to;
    Square (*from)(Square);

    if (stm == Side::WHITE) {
      to = Direction::NORTH;
      from = south;
    } else {
      to = Direction::SOUTH;
      from = north;
    }

    Bitboard bb = pieces[stm][Piece::PAWN],
             last_rank =
                 stm == Side::WHITE ? Bitboards::Rank8 : Bitboards::Rank1,
             third_rank =
                 stm == Side::WHITE ? Bitboards::Rank3 : Bitboards::Rank6,
             single_pushes = bb.shift(to) & ~general_occupancy,
             double_pushes =
                 (single_pushes & third_rank).shift(to) & ~general_occupancy;

    while (single_pushes) {
      Square to = single_pushes.pop_lsb();

      if (last_rank & Bitboard(to))
        for (Piece p :
             {Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN})
          move_list.push_back(Move(from(to), to, false, p));
      else
        move_list.push_back(Move(from(to), to, false));
    }

    while (double_pushes) {
      Square to = double_pushes.pop_lsb();
      move_list.push_back(Move(from(from(to)), to, Special::DOUBLE_PUSH));
    }

    while (bb) {
      Square from = bb.pop_lsb();
      Bitboard attacks =
          pawn_attacks[stm][from] & side_occupancy[opponent(stm)];

      while (attacks) {
        Square to = attacks.pop_lsb();

        if (last_rank & Bitboard(to))
          for (Piece p :
               {Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN})
            move_list.push_back(Move(from, to, true, p));
        else
          move_list.push_back(Move(from, to, true));
      }
    }

    if (en_passant_square) {
      Bitboard attackers =
          pawn_attacks[opponent(stm)][en_passant_square.value()] &
          pieces[stm][Piece::PAWN];

      while (attackers) {
        Square from = attackers.pop_lsb();
        move_list.push_back(
            Move(from, en_passant_square.value(), Special::EN_PASSANT));
      }
    }
  }

  constexpr MoveList pseudolegal_moves() const {
    MoveList moves;

    generate_regular_moves(moves);
    generate_castling_moves(moves);
    generate_pawn_moves(moves);

    return moves;
  };

  constexpr int evaluation() const {
    auto eval_side = [&](Side side) {
      return std::ranges::fold_left(
          std::views::zip_transform(
              [](Bitboard bb, int value) { return bb.popcount() * value; },
              pieces[side], PIECE_VALUES),
          0, std::plus{});
    };

    return eval_side(stm) - eval_side(opponent(stm));
  }

  constexpr bool move_comparator(Move a, Move b) const {
    if (a.is_capture() && b.is_capture()) {
      Piece a_attacker = square_to_piece[a.from()].value(),
            b_attacker = square_to_piece[b.from()].value(),
            a_victim = a.is_en_passant() ? Piece::PAWN
                                         : square_to_piece[a.to()].value(),
            b_victim = b.is_en_passant() ? Piece::PAWN
                                         : square_to_piece[b.to()].value();

      if (a_attacker == b_attacker)
        return a_victim > b_victim;

      return a_attacker < b_attacker;
    }

    return a.is_capture();
  }

  constexpr Move
  bestmove(std::chrono::steady_clock::time_point deadline) const {
    Move best_move_iter, best_move(Square::a1, Square::a1, false);
    int best_value_iter;
    bool timed_out = false;
    size_t nodes = 0;

    static constexpr int INFINITY = 1e9;

    auto time_up = [deadline]() {
      return std::chrono::steady_clock::now() >= deadline;
    };

    auto negamax = [&](this auto self, const Board &board, size_t depth,
                       size_t ply, int alpha, int beta) {
      if (depth == 0)
        return board.evaluation();

      int value = -INFINITY;

      MoveList moves = board.pseudolegal_moves();

      std::ranges::sort(moves,
                        std::bind_front(&Board::move_comparator, &board));

      for (Move move : moves) {
        if (++nodes == 1024) {
          if (time_up()) {
            timed_out = true;
            break;
          }

          nodes = 0;
        }

        Board copy = board;
        copy.make_move(move);

        if (copy.is_legal()) {
          value =
              std::max(value, -self(copy, depth - 1, ply + 1, -beta, -alpha));

          if (timed_out)
            break;

          if (ply == 0 && value > best_value_iter) {
            best_value_iter = value;
            best_move_iter = move;
          }

          if (alpha >= beta)
            break;
        }
      }

      if (value == -INFINITY)
        return board.is_check() ? static_cast<int>(ply) - INFINITY : 0;

      return value;
    };

    for (size_t depth = 1; !time_up(); ++depth) {
      best_value_iter = -INFINITY;

      negamax(*this, depth, 0, -INFINITY, INFINITY);

      if (!timed_out)
        best_move = best_move_iter;
      else
        break;
    }

    return best_move;
  }
};

template <> struct std::formatter<Board> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const Board &board, std::format_context &ctx) const {
    auto out = ctx.out();

    out = std::format_to(out, "\t\tA B C D E F G H\n");

    for (int8_t rank = 7; rank >= 0; --rank) {
      out = std::format_to(out, "\t{}\t", rank + 1);
      for (uint8_t file = 0; file < 8; ++file) {
        out =
            std::format_to(out, "{} ",
                           board.square_to_piece[get_square(rank, file)]
                               .transform([&](Piece piece) {
                                 return piece_to_char(
                                     piece, board.side_occupancy[Side::WHITE] &
                                                    Bitboard(rank, file)
                                                ? Side::WHITE
                                                : Side::BLACK);
                               })
                               .value_or('.'));
      }

      out = std::format_to(out, "\t{}\n", rank + 1);
    }

    std::string castling;
    if (board.castling_rights[Side::WHITE][0])
      castling += 'K';
    if (board.castling_rights[Side::WHITE][1])
      castling += 'Q';
    if (board.castling_rights[Side::BLACK][0])
      castling += 'k';
    if (board.castling_rights[Side::BLACK][1])
      castling += 'q';
    if (castling.empty())
      castling = "-";

    out = std::format_to(
        out, "\n\t\tA B C D E F G H\t\t{} {} {} {}\n",
        board.stm == Side::WHITE ? 'w' : 'b', castling,
        board.en_passant_square
            .transform([](Square square) { return to_string(square); })
            .value_or("-"),
        board.halfmove_clock);

    return out;
  }
};
