#pragma once

#include "attacks.hpp"
#include "enum_array.hpp"
#include "move.hpp"
#include "types.hpp"
#include <cstdint>
#include <print>
#include <ranges>
#include <vector>

using MoveList = std::array<Move, 256>;

struct Board {
  enum_array<Side, enum_array<Piece, Bitboard, NUM_PIECES>, NUM_SIDES> pieces;
  enum_array<Side, Bitboard, NUM_SIDES + 1> side_occupancy;
  enum_array<Square, std::optional<Piece>, NUM_SQUARES> square_to_piece;
  Bitboard general_occupancy;

  uint8_t halfmove_clock;
  std::optional<Square> en_passant_square;
  Side stm;
  enum_array<Side, std::array<bool, 2>, NUM_SIDES> castling_rights;

  constexpr Board() = default;

  constexpr Board(std::string_view fen_string) {
    std::vector<std::string_view> tokens =
        fen_string | std::views::split(' ') |
        std::views::transform(
            [](auto subrange) { return std::string_view(subrange); }) |
        std::ranges::to<std::vector<std::string_view>>();

    pieces = {};
    side_occupancy = {};
    square_to_piece = {};

    uint8_t rank = 7, file = 0;

    for (char c : tokens[0])
      if (c == '/') {
        --rank;
        file = 0;
      } else if (isdigit(c))
        file += c - '0';
      else {
        const Piece piece = char_to_piece(c).value();
        const Side side = 'A' <= c && c <= 'Z' ? Side::WHITE : Side::BLACK;

        pieces[side][piece] |= get_bit(rank, file);
        side_occupancy[side] |= get_bit(rank, file);
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
      pieces[side][piece] |= get_bit(square);
      square_to_piece[square] = piece;
    };

    auto remove_piece = [&](Side side, Piece piece, Square square) {
      pieces[side][piece] &= ~get_bit(square);
      square_to_piece[square] = std::nullopt;
    };

    auto move_piece = [&](Side side, Piece piece, Square from, Square to) {
      remove_piece(side, piece, from);
      add_piece(side, piece, to);
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
        threats |= attacks_bb(p, pop_lsb(bb), general_occupancy);
    }

    Bitboard bb = pieces[side][Piece::PAWN];
    while (bb)
      threats |= pawn_attacks[side][pop_lsb(bb)];

    return threats & ~side_occupancy[side];
  }

  constexpr bool is_attacked(Square square, Side side) const {
    for (Piece p :
         {Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN, Piece::KING})
      if (attacks_bb(p, square, general_occupancy) & pieces[side][p])
        return true;

    return pawn_attacks[opponent(side)][square] & pieces[side][Piece::PAWN];
  }

  constexpr bool is_check() const {
    return is_attacked(static_cast<Square>(std::countr_zero(
                           pieces[opponent(stm)][Piece::KING])),
                       stm);
  }

  constexpr Move *generate_regular_moves(Move *move_list) const {
    for (Piece p : {Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN,
                    Piece::KING}) {
      Bitboard bb = pieces[stm][p];

      while (bb) {
        Square from = pop_lsb(bb);
        Bitboard attacks =
            attacks_bb(p, from, general_occupancy) & ~side_occupancy[stm];

        while (attacks) {
          Square to = pop_lsb(attacks);
          new (move_list++) Move(from, to, square_to_piece[to].has_value());
        }
      }
    }

    return move_list;
  }

  constexpr Move *generate_castling_moves(Move *move_list) const {
    static constexpr enum_array<Side, std::array<Bitboard, 2>, NUM_SIDES>
        castling_free_masks = {0x60, 0xE, 0x6000000000000000,
                               0xE00000000000000};

    static constexpr enum_array<Side, std::array<Bitboard, 2>, NUM_SIDES>
        castling_attack_masks = {0x70, 0x1C, 0x7000000000000000,
                                 0x1c00000000000000};

    Bitboard threats_bb = threats(opponent(stm));
    size_t castling_rank = stm == Side::WHITE ? 0 : 7;
    Square king_square = get_square(castling_rank, 4);

    for (size_t i = 0; i < 2; ++i)
      if (castling_rights[stm][i] &&
          !(general_occupancy & castling_free_masks[stm][i]) &&
          !(threats_bb & castling_attack_masks[stm][i])) {
        new (move_list++)
            Move(king_square, get_square(castling_rank, 6 - 4 * i),
                 i == 0 ? Special::KING_CASTLE : Special::QUEEN_CASTLE);
      }

    return move_list;
  }

  constexpr Move *generate_pawn_moves(Move *move_list) const {
    // Function pointers to disambiguate templated functions
    Bitboard (*to)(Bitboard);
    Square (*from)(Square);

    if (stm == Side::WHITE) {
      to = north;
      from = south;
    } else {
      to = south;
      from = north;
    }

    Bitboard bb = pieces[stm][Piece::PAWN],
             last_rank = ranks[stm == Side::WHITE ? 7 : 0],
             single_pushes = to(bb) & ~general_occupancy,
             double_pushes =
                 to(single_pushes & ranks[stm == Side::WHITE ? 2 : 5]) &
                 ~general_occupancy;

    while (single_pushes) {
      Square to = pop_lsb(single_pushes);

      if (last_rank & get_bit(to))
        for (Piece p :
             {Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN})
          new (move_list++) Move(from(to), to, false, p);
      else
        new (move_list++) Move(from(to), to, false);
    }

    while (double_pushes) {
      Square to = pop_lsb(double_pushes);
      new (move_list++) Move(from(from(to)), to, Special::DOUBLE_PUSH);
    }

    while (bb) {
      Square from = pop_lsb(bb);
      Bitboard attacks =
          pawn_attacks[stm][from] & side_occupancy[opponent(stm)];

      while (attacks) {
        Square to = pop_lsb(attacks);

        if (last_rank & get_bit(to))
          for (Piece p :
               {Piece::KNIGHT, Piece::BISHOP, Piece::ROOK, Piece::QUEEN})
            new (move_list++) Move(from, to, true, p);
        else
          new (move_list++) Move(from, to, true);
      }
    }

    if (en_passant_square) {
      Bitboard attackers =
          pawn_attacks[opponent(stm)][en_passant_square.value()] &
          pieces[stm][Piece::PAWN];

      while (attackers) {
        Square from = pop_lsb(attackers);
        new (move_list++)
            Move(from, en_passant_square.value(), Special::EN_PASSANT);
      }
    }

    return move_list;
  }

  constexpr std::pair<MoveList, size_t> generate_pseudolegal_moves() const {
    MoveList moves;

    Move *ptr = moves.data();

    ptr = generate_regular_moves(ptr);
    ptr = generate_castling_moves(ptr);
    ptr = generate_pawn_moves(ptr);

    return make_pair(moves, ptr - moves.data());
  };

  constexpr std::pair<MoveList, size_t> generate_legal_moves() const {
    auto [moves, len] = generate_pseudolegal_moves();

    return std::make_pair(
        moves, std::partition(moves.begin(), moves.begin() + len, [&](Move m) {
                 Board copy = *this;
                 copy.make_move(m);

                 return !copy.is_check();
               }) - moves.begin());
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
                                                    get_bit(rank, file)
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
