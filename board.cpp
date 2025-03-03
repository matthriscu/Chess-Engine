#include "board.hpp"
#include "piece.hpp"
#include <cstring>
#include <print>
#include <sstream>

Board::Board() { memset(this, 0, sizeof(*this)); }

Board::Board(const std::string &fen_string) {
  std::string board, side, castling, en_passant, halfmove;

  {
    std::stringstream ss(fen_string);
    ss >> board >> side >> castling >> en_passant >> halfmove;
  }

  uint8_t rank = 7, file = 0;
  memset(pieces, 0, sizeof(pieces));

  for (char c : board)
    if (c == '/') {
      --rank;
      file = 0;
    } else if (isdigit(c))
      file += c - '0';
    else {
      pieces['A' <= c && c <= 'Z' ? WHITE : BLACK][char_to_piece(c)] |=
          get_bit(file, rank);
      ++file;
    }

  side_to_move = side == "w" ? WHITE : BLACK;
  wk_castling = castling.contains('K');
  wq_castling = castling.contains('Q');
  bk_castling = castling.contains('k');
  bq_castling = castling.contains('q');
  en_passant_file = en_passant != "-" ? en_passant[0] - 'a' : 8;
  halfmove_clock = std::stoi(halfmove);
}

void Board::print() {
  auto get_piece = [&](auto rank, auto file) {
    for (uint8_t side = 0; side < NUM_SIDES; ++side)
      for (uint8_t piece = 0; piece < NUM_PIECES; ++piece)
        if (pieces[side][piece] & get_bit(file, rank))
          return piece_to_char(static_cast<Piece>(piece),
                               static_cast<Side>(side));

    return '.';
  };

  std::puts("\t\tA B C D E F G H\n");

  for (int8_t rank = 7; rank >= 0; --rank) {
    std::print("\t{}\t", rank);

    for (uint8_t file = 0; file < 8; ++file)
      std::print("{} ", get_piece(rank, file));

    std::println("\t{}", rank);
  }

  std::string castling;

  if (wk_castling)
    castling += 'K';

  if (wq_castling)
    castling += 'Q';

  if (bk_castling)
    castling += 'k';

  if (bq_castling)
    castling += 'q';

  if (castling.empty())
    castling = "-";

  std::println(
      "\n\t\tA B C D E F G H\t\t{} {} {} {}", side_to_move == WHITE ? 'w' : 'b',
      castling,
      static_cast<char>(en_passant_file != 8 ? 'a' + en_passant_file : '-'),
      halfmove_clock);
}

std::vector<Move> generate_moves() { return {}; }
