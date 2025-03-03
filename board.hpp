#pragma once

#include "helper.hpp"
#include "move.hpp"
#include "piece.hpp"
#include <cstdint>
#include <string>
#include <vector>

struct Board {
  Bitboard pieces[NUM_SIDES + 1][NUM_PIECES + 1];
  Piece square_to_piece[64];

  uint8_t halfmove_clock;
  uint8_t en_passant_file;
  uint8_t side_to_move;
  uint8_t wk_castling;
  uint8_t wq_castling;
  uint8_t bk_castling;
  uint8_t bq_castling;

  Board();
  Board(const std::string &fen_string);
  void print();
  std::vector<Move> generate_moves();
};
