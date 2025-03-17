#pragma once

#include "bitboard.hpp"
#include "move.hpp"
#include "types.hpp"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <print>
#include <ranges>

using move_list = std::array<Move, 256>;

struct Board {
  std::array<std::array<Bitboard, NUM_PIECES>, NUM_SIDES> pieces;
  std::array<Bitboard, NUM_SIDES + 1> occupancy;
  std::array<std::pair<Piece, Side>, 64> square_to_piece;

  uint8_t halfmove_clock;
  std::optional<Square> en_passant_square;
  Side stm;
  std::array<std::array<bool, 2>, NUM_SIDES>
      castling_rights; // 0 = Kingside, 1 = Queenside

  constexpr Board(const char *fen_string) {
    std::array<std::string, 5> tokens;
    uint8_t current_token = 0;

    for (; *fen_string; ++fen_string)
      if (*fen_string != ' ')
        tokens[current_token] += *fen_string;
      else if (++current_token >= tokens.size())
        break;

    pieces = {};
    occupancy = {};
    std::ranges::fill(square_to_piece, std::make_pair(NUM_PIECES, NUM_SIDES));

    uint8_t rank = 7, file = 0;

    for (char c : tokens[0])
      if (c == '/') {
        --rank;
        file = 0;
      } else if (isdigit(c))
        file += c - '0';
      else {
        const Piece piece = char_to_piece(c);
        const Side side = 'A' <= c && c <= 'Z' ? WHITE : BLACK;

        pieces[side][piece] |= get_bit(rank, file);
        occupancy[side] |= get_bit(rank, file);
        square_to_piece[get_square(rank, file)] = std::make_pair(piece, side);

        ++file;
      }

    occupancy[ALL_SIDES] = occupancy[WHITE] | occupancy[BLACK];
    stm = tokens[1] == "w" ? WHITE : BLACK;
    castling_rights[WHITE][0] = tokens[2].contains('K');
    castling_rights[WHITE][1] = tokens[2].contains('Q');
    castling_rights[BLACK][0] = tokens[2].contains('k');
    castling_rights[BLACK][1] = tokens[2].contains('q');
    en_passant_square = from_string(tokens[3]);
    halfmove_clock = std::stoul(tokens[4]);
  }

  constexpr void make_move(Move m) {
    auto add_piece = [&](Side side, Piece piece, Square square) {
      pieces[side][piece] |= get_bit(square);
      square_to_piece[square] = std::make_pair(piece, side);
    };

    auto remove_piece = [&](Side side, Piece piece, Square square) {
      pieces[side][piece] &= ~get_bit(square);
      square_to_piece[square] = {NUM_PIECES, NUM_SIDES};
    };

    auto move_piece = [&](Side side, Piece piece, Square from, Square to) {
      remove_piece(side, piece, from);
      add_piece(side, piece, to);
    };

    Piece moved_piece = square_to_piece[m.from].first,
          captured_piece = square_to_piece[m.to].first;

    if (m.is_en_passant())
      remove_piece(opposite_side(stm), PAWN,
                   stm == WHITE ? south(en_passant_square.value())
                                : north(en_passant_square.value()));
    else if (m.is_capture)
      remove_piece(opposite_side(stm), captured_piece, m.to);

    move_piece(stm, moved_piece, m.from, m.to);

    if (moved_piece == KING)
      castling_rights[stm] = {};

    if (m.from == h1 || m.to == h1)
      castling_rights[WHITE][0] = false;
    else if (m.from == a1 || m.to == a1)
      castling_rights[WHITE][1] = false;

    if (m.from == h8 || m.to == h8)
      castling_rights[BLACK][0] = false;
    else if (m.from == a8 || m.to == a8)
      castling_rights[BLACK][1] = false;

    if (m.is_promotion) {
      remove_piece(stm, PAWN, m.to);
      add_piece(stm, static_cast<Piece>(m.special + 1), m.to);
    }

    if (m.is_castle()) {
      if (m.to == g1)
        move_piece(stm, ROOK, h1, f1);
      else if (m.to == c1)
        move_piece(stm, ROOK, a1, d1);
      else if (m.to == g8)
        move_piece(stm, ROOK, h8, f8);
      else if (m.to == c8)
        move_piece(stm, ROOK, a8, d8);
    }

    for (Side side : {WHITE, BLACK}) {
      occupancy[side] = 0;

      for (Piece piece : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING})
        occupancy[side] |= pieces[side][piece];
    }

    if (m.is_double_push())
      en_passant_square =
          std::make_optional(stm == WHITE ? south(m.to) : north(m.to));
    else
      en_passant_square = std::nullopt;

    occupancy[ALL_SIDES] = occupancy[WHITE] | occupancy[BLACK];
    stm = opposite_side(stm);
    halfmove_clock = moved_piece == PAWN || captured_piece != NUM_PIECES
                         ? 0
                         : halfmove_clock + 1;
  }

  constexpr Bitboard threats(Side side) const {
    Bitboard threats = 0;

    for (Piece p : {KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
      Bitboard bb = pieces[side][p];

      while (bb)
        threats |= attacks_bb(p, pop_lsb(bb), occupancy[ALL_SIDES]);
    }

    Bitboard bb = pieces[side][PAWN];
    while (bb)
      threats |= pawn_attacks[side][pop_lsb(bb)];

    return threats & ~occupancy[side];
  }

  constexpr bool is_attacked(Square square, Side side) const {
    for (Piece p : {KNIGHT, BISHOP, ROOK, QUEEN, KING})
      if (attacks_bb(p, square, occupancy[ALL_SIDES]) & pieces[side][p])
        return true;

    return pawn_attacks[opposite_side(side)][square] & pieces[side][PAWN];
  }

  constexpr bool is_free(Square square) const {
    return !(occupancy[ALL_SIDES] & get_bit(square));
  }

  constexpr Move *generate_regular_moves(Move *move_list) const {
    for (Piece p : {KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
      Bitboard bb = pieces[stm][p];

      while (bb) {
        Square from = pop_lsb(bb);
        Bitboard attacks =
            attacks_bb(p, from, occupancy[ALL_SIDES]) & ~occupancy[stm];

        while (attacks) {
          Square to = pop_lsb(attacks);
          new (move_list++)
              Move(from, to, false, square_to_piece[to].first != NUM_PIECES);
        }
      }
    }

    return move_list;
  }

  constexpr Move *generate_castling_moves(Move *move_list) const {
    static constexpr std::array<std::array<Bitboard, 2>, NUM_SIDES>
        castling_free_masks = {0x60, 0xE, 0x6000000000000000,
                               0xE00000000000000};

    static constexpr std::array<std::array<Bitboard, 2>, NUM_SIDES>
        castling_attack_masks = {0x70, 0x1C, 0x7000000000000000,
                                 0x1c00000000000000};

    Bitboard threats_bb = threats(opposite_side(stm));
    size_t castling_rank = stm == WHITE ? 0 : 7;
    Square king_square = get_square(castling_rank, 4);

    for (size_t i = 0; i < 2; ++i)
      if (castling_rights[stm][i] &&
          !(occupancy[ALL_SIDES] & castling_free_masks[stm][i]) &&
          !(threats_bb & castling_attack_masks[stm][i]))
        new (move_list++)
            Move(king_square, get_square(castling_rank, 6 - 4 * i), false,
                 false, static_cast<Special>(KING_CASTLE + i));

    return move_list;
  }

  constexpr Move *generate_pawn_moves(Move *move_list) const {
    // Function pointers to disambiguate templated functions
    Bitboard (*to)(Bitboard);
    Square (*from)(Square);

    if (stm == WHITE) {
      to = north;
      from = south;
    } else {
      to = south;
      from = north;
    }

    Bitboard bb = pieces[stm][PAWN], last_rank = ranks[stm == WHITE ? 7 : 0],
             single_pushes = to(bb) & ~occupancy[ALL_SIDES],
             double_pushes = to(single_pushes & ranks[stm == WHITE ? 2 : 5]) &
                             ~occupancy[ALL_SIDES];

    while (single_pushes) {
      Square to = pop_lsb(single_pushes);

      if (last_rank & get_bit(to))
        for (Piece p : {KNIGHT, BISHOP, ROOK, QUEEN})
          new (move_list++) Move(from(to), to, true, false, p - 1);
      else
        new (move_list++) Move(from(to), to, false, false);
    }

    while (double_pushes) {
      Square to = pop_lsb(double_pushes);
      new (move_list++) Move(from(from(to)), to, false, false, DOUBLE_PUSH);
    }

    while (bb) {
      Square from = pop_lsb(bb);
      Bitboard attacks =
          pawn_attacks[stm][from] & occupancy[opposite_side(stm)];

      while (attacks) {
        Square to = pop_lsb(attacks);

        if (last_rank & get_bit(to))
          for (Piece p : {KNIGHT, BISHOP, ROOK, QUEEN})
            new (move_list++) Move(from, to, true, true, p - 1);
        else
          new (move_list++) Move(from, to, false, true);
      }
    }

    if (en_passant_square) {
      Bitboard attackers =
          pawn_attacks[opposite_side(stm)][en_passant_square.value()] &
          pieces[stm][PAWN];

      while (attackers) {
        Square from = pop_lsb(attackers);
        new (move_list++)
            Move(from, en_passant_square.value(), false, true, EN_PASSANT);
      }
    }

    return move_list;
  }

  constexpr bool is_legal(Move move) const {
    Board copy = *this;
    copy.make_move(move);

    return !copy.is_attacked(
        static_cast<Square>(std::countr_zero(copy.pieces[stm][KING])),
        opposite_side(stm));
  }

  constexpr std::pair<move_list, size_t> generate_pseudolegal_moves() const {
    move_list moves;

    Move *ptr = moves.data();

    ptr = generate_regular_moves(ptr);
    ptr = generate_castling_moves(ptr);
    ptr = generate_pawn_moves(ptr);

    return make_pair(moves, ptr - moves.data());
  };

  constexpr std::pair<move_list, size_t> generate_legal_moves() const {
    using namespace std::placeholders;

    auto [moves, size] = generate_pseudolegal_moves();

    size = std::partition(moves.begin(), moves.begin() + size,
                          std::bind(&Board::is_legal, *this, _1)) -
           moves.begin();

    return make_pair(moves, size);
  }
};

template <> struct std::formatter<Board> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const Board &board, std::format_context &ctx) const {
    auto out = ctx.out();

    out = std::format_to(out, "\t\tA B C D E F G H\n");

    for (int8_t rank = 7; rank >= 0; --rank) {
      out = std::format_to(out, "\t{}\t", rank + 1);
      for (uint8_t file = 0; file < 8; ++file)
        out = std::format_to(
            out, "{} ",
            std::apply(piece_to_char,
                       board.square_to_piece[get_square(rank, file)]));

      out = std::format_to(out, "\t{}\n", rank + 1);
    }

    std::string castling;
    if (board.castling_rights[WHITE][0])
      castling += 'K';
    if (board.castling_rights[WHITE][1])
      castling += 'Q';
    if (board.castling_rights[BLACK][0])
      castling += 'k';
    if (board.castling_rights[BLACK][1])
      castling += 'q';
    if (castling.empty())
      castling = "-";

    out = std::format_to(
        out, "\n\t\tA B C D E F G H\t\t{} {} {} {}\n",
        board.stm == WHITE ? 'w' : 'b', castling,
        board.en_passant_square
            .transform([](Square square) { return to_string(square); })
            .value_or("-"),
        board.halfmove_clock);

    return out;
  }
};
