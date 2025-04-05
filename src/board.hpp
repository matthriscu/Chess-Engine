#pragma once

#include "attacks.hpp"
#include "hashing.hpp"
#include "movelist.hpp"
#include "util.hpp"
#include <cassert>

struct Board {
  Sides::Array<Pieces::Array<Bitboard>> pieces;
  Sides::Array<Bitboard> side_occupancy;
  Squares::Array<Piece> square_to_piece;
  Bitboard general_occupancy;
  int halfmove_clock;
  Square ep_square;
  Side stm;
  Sides::Array<std::array<bool, 2>> castling_rights;
  uint64_t zobrist;

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
        Piece piece(c);

        pieces[isupper(c) ? Sides::WHITE : Sides::BLACK][piece] |=
            Bitboard(rank, file);
        square_to_piece[Square(rank, file)] = piece;

        ++file;
      }

    for (Side side : Sides::ALL)
      for (Piece piece : Pieces::ALL)
        side_occupancy[side] |= pieces[side][piece];

    general_occupancy =
        side_occupancy[Sides::WHITE] | side_occupancy[Sides::BLACK];
    stm = tokens[1] == "w" ? Sides::WHITE : Sides::BLACK;
    castling_rights[Sides::WHITE][0] = tokens[2].contains('K');
    castling_rights[Sides::WHITE][1] = tokens[2].contains('Q');
    castling_rights[Sides::BLACK][0] = tokens[2].contains('k');
    castling_rights[Sides::BLACK][1] = tokens[2].contains('q');
    ep_square = Square(tokens[3]);
    std::from_chars(tokens[4].begin(), tokens[4].end(), halfmove_clock);
    zobrist = hash();
  }

  constexpr void make_move(Move m) {
    auto add_piece = [&](Side side, Piece piece, Square square) {
      pieces[side][piece] |= Bitboard(square);
      square_to_piece[square] = piece;
      zobrist ^= square_rands[square][piece][side];
    };

    auto remove_piece = [&](Side side, Piece piece, Square square) {
      pieces[side][piece] &= ~Bitboard(square);
      square_to_piece[square] = Pieces::NONE;
      zobrist ^= square_rands[square][piece][side];
    };

    auto move_piece = [&](Side side, Piece piece, Square from, Square to) {
      pieces[side][piece] ^= Bitboard(from) | Bitboard(to);
      square_to_piece[from] = Pieces::NONE;
      square_to_piece[to] = piece;
      zobrist ^=
          square_rands[from][piece][side] ^ square_rands[to][piece][side];
    };

    Piece moved_piece = square_to_piece[m.from()];

    if (m.is_en_passant())
      remove_piece(~stm, Pieces::PAWN,
                   ep_square.shift(stm == Sides::WHITE ? Direction::SOUTH
                                                       : Direction::NORTH));
    else if (m.is_capture())
      remove_piece(~stm, square_to_piece[m.to()], m.to());

    move_piece(stm, moved_piece, m.from(), m.to());

    if (moved_piece == Pieces::KING)
      for (int i = 0; i < 2; ++i)
        if (castling_rights[stm][i]) {
          castling_rights[stm][i] = false;
          zobrist ^= castling_rands[stm][i];
        }

    if (castling_rights[Sides::WHITE][0] &&
        (m.from() == Squares::H1 || m.to() == Squares::H1)) {
      castling_rights[Sides::WHITE][0] = false;
      zobrist ^= castling_rands[Sides::WHITE][0];
    } else if (castling_rights[Sides::WHITE][1] &&
               (m.from() == Squares::A1 || m.to() == Squares::A1)) {
      castling_rights[Sides::WHITE][1] = false;
      zobrist ^= castling_rands[Sides::WHITE][1];
    }

    if (castling_rights[Sides::BLACK][0] &&
        (m.from() == Squares::H8 || m.to() == Squares::H8)) {
      castling_rights[Sides::BLACK][0] = false;
      zobrist ^= castling_rands[Sides::BLACK][0];
    } else if (castling_rights[Sides::BLACK][1] &&
               (m.from() == Squares::A8 || m.to() == Squares::A8)) {
      castling_rights[Sides::BLACK][1] = false;
      zobrist ^= castling_rands[Sides::BLACK][1];
    }

    if (m.is_promotion()) {
      remove_piece(stm, Pieces::PAWN, m.to());
      add_piece(stm, m.promoted_to(), m.to());
    }

    if (m.is_castle()) {
      if (m.to() == Squares::G1)
        move_piece(stm, Pieces::ROOK, Squares::H1, Squares::F1);
      else if (m.to() == Squares::C1)
        move_piece(stm, Pieces::ROOK, Squares::A1, Squares::D1);
      else if (m.to() == Squares::G8)
        move_piece(stm, Pieces::ROOK, Squares::H8, Squares::F8);
      else if (m.to() == Squares::C8)
        move_piece(stm, Pieces::ROOK, Squares::A8, Squares::D8);
    }

    if (ep_square != Squares::NONE)
      zobrist ^= ep_rands[ep_square.file()];

    ep_square = Squares::NONE;

    if (m.is_double_push()) {
      Square ep_square_candidate = m.to().shift(
          stm == Sides::WHITE ? Direction::SOUTH : Direction::NORTH);

      if (pieces[~stm][Pieces::PAWN] & pawn_attacks[stm][ep_square_candidate]) {
        ep_square = ep_square_candidate;
        zobrist ^= ep_rands[ep_square.file()];
      }
    }

    side_occupancy = {};

    for (Side side : Sides::ALL)
      for (Piece piece : Pieces::ALL)
        side_occupancy[side] |= pieces[side][piece];

    general_occupancy =
        side_occupancy[Sides::WHITE] | side_occupancy[Sides::BLACK];
    halfmove_clock =
        moved_piece == Pieces::PAWN || m.is_capture() ? 0 : halfmove_clock + 1;
    stm = ~stm;
    zobrist ^= stm_rand;
  }

  // Does not account for pins
  constexpr Bitboard threats(Side side) const {
    Bitboard threats;

    for (Piece p : std::views::drop(Pieces::ALL, 1)) {
      Bitboard bb = pieces[side][p];

      while (bb)
        threats |= attacks_bb(p, bb.pop_lsb(), general_occupancy);
    }

    Bitboard bb = pieces[side][Pieces::PAWN];
    while (bb)
      threats |= pawn_attacks[side][bb.pop_lsb()];

    return threats & ~side_occupancy[side];
  }

  // Does not account for pins
  constexpr bool is_attacked(Square square, Side side) const {
    for (Piece p : std::views::drop(Pieces::ALL, 1))
      if (attacks_bb(p, square, general_occupancy) & pieces[side][p])
        return true;

    return pawn_attacks[~side][square] & pieces[side][Pieces::PAWN];
  }

  constexpr bool is_legal() const {
    return !is_attacked(Square(pieces[~stm][Pieces::KING]), stm);
  }

  constexpr bool is_check() const {
    return is_attacked(Square(pieces[stm][Pieces::KING]), ~stm);
  }

  constexpr bool is_draw() const {
    if (halfmove_clock >= 50)
      return true;

    if (general_occupancy.popcount() > 3)
      return false;

    for (Side side : Sides::ALL)
      for (Piece piece : {Pieces::PAWN, Pieces::ROOK, Pieces::QUEEN})
        if (pieces[side][piece])
          return false;

    return true;
  }

  constexpr void generate_regular_moves(MoveList &move_list) const {
    for (Piece p : std::views::drop(Pieces::ALL, 1)) {
      Bitboard bb = pieces[stm][p];

      while (bb) {
        Square from = bb.pop_lsb();

        Bitboard attacks =
            attacks_bb(p, from, general_occupancy) & ~side_occupancy[stm];

        while (attacks) {
          Square to = attacks.pop_lsb();
          move_list.add(from, to, square_to_piece[to] != Pieces::NONE);
        }
      }
    }
  }

  constexpr void generate_castling_moves(MoveList &move_list) const {
    static constexpr Sides::Array<std::array<Bitboard, 2>> free_masks = {
        0x60, 0xE, 0x6000000000000000, 0xE00000000000000};

    static constexpr Sides::Array<std::array<Bitboard, 2>> attack_masks = {
        0x70, 0x1C, 0x7000000000000000, 0x1c00000000000000};

    Bitboard threats_bb = threats(~stm);
    size_t castling_rank = stm == Sides::WHITE ? 0 : 7;
    Square king_square(castling_rank, 4);

    for (size_t i = 0; i < 2; ++i)
      if (castling_rights[stm][i] &&
          !(general_occupancy & free_masks[stm][i]) &&
          !(threats_bb & attack_masks[stm][i]))
        move_list.add(king_square, Square(castling_rank, 6 - 4 * i),
                      i == 0 ? Special::KING_CASTLE : Special::QUEEN_CASTLE);
  }

  constexpr void generate_pawn_moves(MoveList &move_list) const {
    Direction from, to;

    if (stm == Sides::WHITE) {
      to = Direction::NORTH;
      from = Direction::SOUTH;
    } else {
      to = Direction::SOUTH;
      from = Direction::NORTH;
    }

    Bitboard bb = pieces[stm][Pieces::PAWN],
             last_rank =
                 stm == Sides::WHITE ? Bitboards::Rank8 : Bitboards::Rank1,
             third_rank =
                 stm == Sides::WHITE ? Bitboards::Rank3 : Bitboards::Rank6,
             single_pushes = bb.shift(to) & ~general_occupancy,
             double_pushes =
                 (single_pushes & third_rank).shift(to) & ~general_occupancy;

    while (single_pushes) {
      Square to = single_pushes.pop_lsb();

      if (last_rank & Bitboard(to))
        for (Piece p :
             {Pieces::KNIGHT, Pieces::BISHOP, Pieces::ROOK, Pieces::QUEEN}) {
          move_list.add(to.shift(from), to, false, p);
        }
      else
        move_list.add(to.shift(from), to, false);
    }

    while (double_pushes) {

      Square to = double_pushes.pop_lsb();
      move_list.add(to.shift(from).shift(from), to, Special::DOUBLE_PUSH);
    }

    while (bb) {
      Square from = bb.pop_lsb();
      Bitboard attacks = pawn_attacks[stm][from] & side_occupancy[~stm];

      while (attacks) {
        Square to = attacks.pop_lsb();

        if (last_rank & Bitboard(to))
          for (Piece p :
               {Pieces::KNIGHT, Pieces::BISHOP, Pieces::ROOK, Pieces::QUEEN})
            move_list.add(from, to, true, p);
        else
          move_list.add(from, to, true);
      }
    }

    if (ep_square != Squares::NONE) {
      Bitboard attackers =
          pawn_attacks[~stm][ep_square] & pieces[stm][Pieces::PAWN];

      while (attackers) {
        Square from = attackers.pop_lsb();
        move_list.add(from, ep_square, Special::EN_PASSANT);
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

  constexpr uint64_t hash() const {
    uint64_t hash = 0;

    for (auto [square, piece] : std::views::zip(Squares::ALL, square_to_piece))
      if (piece != Pieces::NONE) {
        Side side = pieces[Sides::WHITE][piece] & Bitboard(square)
                        ? Sides::WHITE
                        : Sides::BLACK;
        hash ^= square_rands[square][piece][side];
      }

    for (Side side : Sides::ALL)
      for (int i = 0; i < 2; ++i)
        if (castling_rights[side][i])
          hash ^= castling_rands[side][i];

    if (ep_square != Squares::NONE)
      hash ^= ep_rands[ep_square.file()];

    if (stm == Sides::BLACK)
      hash ^= stm_rand;

    return hash;
  }
};

template <> struct std::formatter<Board> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const Board &board, std::format_context &ctx) const {
    auto out = ctx.out();

    out = std::format_to(out, "\t\tA B C D E F G H\n");

    for (int rank = 7; rank >= 0; --rank) {
      out = std::format_to(out, "\t{}\t", rank + 1);
      for (int file = 0; file < 8; ++file) {
        out = std::format_to(out, "{} ",
                             board.square_to_piece[Square(rank, file)]);
      }

      out = std::format_to(out, "\t{}\n", rank + 1);
    }

    std::string castling;
    if (board.castling_rights[Sides::WHITE][0])
      castling += 'K';
    if (board.castling_rights[Sides::WHITE][1])
      castling += 'Q';
    if (board.castling_rights[Sides::BLACK][0])
      castling += 'k';
    if (board.castling_rights[Sides::BLACK][1])
      castling += 'q';
    if (castling.empty())
      castling = "-";

    out = std::format_to(out, "\n\t\tA B C D E F G H\t\t{} {} {} {}\n",
                         board.stm == Sides::WHITE ? 'w' : 'b', castling,
                         board.ep_square, board.halfmove_clock);

    return out;
  }
};
