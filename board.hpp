#pragma once

#include "bitboard.hpp"
#include "move.hpp"
#include "types.hpp"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <print>
#include <ranges>
#include <vector>

struct Board {
  std::array<std::array<Bitboard, NUM_PIECES>, NUM_SIDES> pieces;
  std::array<Bitboard, NUM_SIDES + 1> occupancy;
  std::array<std::pair<Piece, Side>, NUM_SQUARES> square_to_piece;

  uint8_t halfmove_clock;
  Square en_passant_square;
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

    std::for_each(pieces.begin(), pieces.end(), [](auto &piece) {
      std::fill(piece.begin(), piece.end(), 0);
    });
    std::fill(square_to_piece.begin(), square_to_piece.end(),
              std::make_pair(NUM_PIECES, NUM_SIDES));

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

  void make_move(Move move);
  void unmake_move(Move move);

  bool is_attacked(Square square) {
    for (Piece p : {KNIGHT, BISHOP, ROOK, QUEEN, KING})
      if (attacks_bb(p, square, occupancy[ALL_SIDES]) &
          pieces[opposite_side(stm)][p])
        return true;

    return pawn_attacks[stm][square] & pieces[opposite_side(stm)][PAWN];
  }

  bool is_free(Square square) {
    return occupancy[ALL_SIDES] & ~get_bit(square);
  }

  constexpr void get_regular_moves(std::vector<Move> &moves) {
    for (Piece p : {KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
      Bitboard bb = pieces[stm][p];

      while (bb) {
        Square from = pop_lsb(bb);
        Bitboard attacks =
            attacks_bb(p, from, occupancy[ALL_SIDES]) & ~occupancy[stm];

        while (attacks)
          moves.emplace_back(from, pop_lsb(attacks));
      }
    }
  }

  constexpr void get_castling_moves(std::vector<Move> &moves) {
    using namespace std::placeholders;
    namespace rng = std::ranges;
    namespace rv = std::views;

    static std::array<std::vector<size_t>, 2> castling_files{
        std::vector{5UZ, 6UZ}, std::vector{2UZ, 3UZ, 4UZ}};
    size_t castling_rank = stm == WHITE ? 0 : 7;
    Square king_square = get_square(castling_rank, 4);

    if (!is_attacked(king_square))
      for (size_t i = 0; i < 2; ++i) {
        auto relevant_squares = rv::transform(
            castling_files[i],
            std::bind(&get_square<size_t, size_t>, castling_rank, _1));

        if (castling_rights[stm][i] &&
            rng::all_of(relevant_squares,
                        std::bind(&Board::is_free, this, _1)) &&
            !rng::any_of(relevant_squares,
                         std::bind(&Board::is_attacked, this, _1)))
          moves.emplace_back(king_square, relevant_squares[1], CASTLING);
      }
  }

  constexpr void get_pawn_moves(std::vector<Move> &moves) {
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

    Bitboard bb = pieces[stm][PAWN],
             promotable = bb & ranks[stm == WHITE ? 6 : 1],
             single_pushes = to(bb) & ~occupancy[ALL_SIDES],
             double_pushes = to(single_pushes & ranks[stm == WHITE ? 2 : 5]) &
                             ~occupancy[ALL_SIDES];

    while (single_pushes) {
      Square to = pop_lsb(single_pushes);

      if (promotable & get_bit(to))
        for (Piece p : {KNIGHT, BISHOP, ROOK, QUEEN})
          moves.emplace_back(from(to), to, p);
      else
        moves.emplace_back(from(to), to);
    }

    while (double_pushes) {
      Square to = pop_lsb(double_pushes);
      moves.emplace_back(from(from(to)), to);
    }

    while (bb) {
      Square from = pop_lsb(bb);
      Bitboard attacks =
          pawn_attacks[stm][from] & occupancy[opposite_side(stm)];

      while (attacks) {
        Square to = pop_lsb(attacks);

        if (promotable & get_bit(to))
          for (Piece p : {KNIGHT, BISHOP, ROOK, QUEEN})
            moves.emplace_back(from, to, p);
        else
          moves.emplace_back(from, to);
      }
    }

    if (en_passant_square != NUM_SQUARES) {
      Bitboard attackers =
          pawn_attacks[WHITE][to(en_passant_square)] & pieces[stm][PAWN];

      while (attackers) {
        Square from = pop_lsb(attackers);
        moves.emplace_back(from, en_passant_square, EN_PASSANT);
      }
    }
  }

  constexpr std::vector<Move> generate_moves() {
    std::vector<Move> moves;

    get_regular_moves(moves);
    get_castling_moves(moves);
    get_pawn_moves(moves);

    return moves;
  };
};

template <> struct std::formatter<Board> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const Board &board, std::format_context &ctx) const {
    auto out = ctx.out();

    out = std::format_to(out, "\t\tA B C D E F G H\n");

    for (int8_t rank = 7; rank >= 0; --rank) {
      out = std::format_to(out, "\t{}\t", rank);
      for (uint8_t file = 0; file < 8; ++file)
        out = std::format_to(
            out, "{} ",
            std::apply(piece_to_char,
                       board.square_to_piece[get_square(rank, file)]));

      out = std::format_to(out, "\t{}\n", rank);
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

    out = std::format_to(out, "\n\t\tA B C D E F G H\t\t{} {} {} {}\n",
                         board.stm == WHITE ? 'w' : 'b', castling,
                         to_string(board.en_passant_square),
                         board.halfmove_clock);

    return out;
  }
};
