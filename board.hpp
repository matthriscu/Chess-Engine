#pragma once

#include "helper.hpp"
#include "move.hpp"
#include <cstdint>
#include <print>
#include <string>
#include <vector>

struct Board {
  std::array<std::array<Bitboard, NUM_PIECES + 1>, NUM_SIDES + 1> pieces{};
  std::array<std::array<std::pair<Piece, Side>, 8>, 8> square_to_piece{};

  uint8_t halfmove_clock = 0, en_passant_file = 0;
  Side side_to_move = WHITE;
  bool wk_castling = false, wq_castling = false, bk_castling = false,
       bq_castling = false;

  constexpr Board(const char *fen_string) {
    std::array<std::string, 5> tokens;
    uint8_t current_token = 0;

    for (; *fen_string; ++fen_string)
      if (*fen_string != ' ')
        tokens[current_token] += *fen_string;
      else if (++current_token >= tokens.size())
        break;

    std::fill_n(&pieces[0][0], (NUM_PIECES + 1) * (NUM_SIDES + 1), 0);
    std::fill_n(&square_to_piece[0][0], 64,
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
        square_to_piece[rank][file] = std::make_pair(piece, side);

        ++file;
      }

    side_to_move = tokens[1] == "w" ? WHITE : BLACK;
    wk_castling = tokens[2].contains('K');
    wq_castling = tokens[2].contains('Q');
    bk_castling = tokens[2].contains('k');
    bq_castling = tokens[2].contains('q');
    en_passant_file = tokens[3] != "-" ? tokens[3][0] - 'a' : 8;
    halfmove_clock = std::stoul(tokens[4]);
  }

  std::vector<Move> generate_moves();
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
            std::apply(piece_to_char, board.square_to_piece[rank][file]));

      out = std::format_to(out, "\t{}\n", rank);
    }

    std::string castling;
    if (board.wk_castling)
      castling += 'K';
    if (board.wq_castling)
      castling += 'Q';
    if (board.bk_castling)
      castling += 'k';
    if (board.bq_castling)
      castling += 'q';
    if (castling.empty())
      castling = "-";

    out = std::format_to(
        out, "\n\t\tA B C D E F G H\t\t{} {} {:c} {}\n",
        board.side_to_move == WHITE ? 'w' : 'b', castling,
        board.en_passant_file != 8 ? 'a' + board.en_passant_file : '-',
        board.halfmove_clock);

    return out;
  }
};
