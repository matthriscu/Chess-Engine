#pragma once
#include "board.hpp"

template <size_t SIZE> struct NibbleArray {
  std::array<uint8_t, (SIZE + 1) / 2> data;

  void set(size_t index, uint8_t value) {
    data[index / 2] = index % 2 == 0
                          ? (data[index / 2] & 0xF0) | (value & 0x0F)
                          : (data[index / 2] & 0x0F) | ((value & 0x0F) << 4);
  }
};

struct MarlinFormat {
  const uint64_t occupancy;
  NibbleArray<32> pieces;
  const uint8_t ep_square;
  const uint8_t halfmove_clock;
  const uint16_t fullmove_clock = 1; // Not implemented
  const int16_t eval = 0;
  uint8_t wdl;
  const uint8_t padding = 0;

  MarlinFormat(const Board &board)
      : occupancy(board.general_occupancy.raw()), pieces{},
        ep_square(board.ep_square.raw() |
                  (board.stm == Sides::WHITE ? 0 : (1 << 7))),
        halfmove_clock(board.halfmove_clock) {
    Bitboard bb = board.general_occupancy;
    int index = 0;

    while (bb) {
      Square square = bb.pop_lsb();
      uint8_t piece = board.square_to_piece[square].raw();

      if (piece == 3 &&
          ((square == Squares::A1 && board.castling_rights[Sides::WHITE][1]) ||
           (square == Squares::A8 && board.castling_rights[Sides::BLACK][1]) ||
           (square == Squares::H1 && board.castling_rights[Sides::WHITE][0]) ||
           (square == Squares::H8 && board.castling_rights[Sides::BLACK][0])))
        piece = 6;

      if (board.side_occupancy[Sides::BLACK] & Bitboard(square))
        piece |= 8;

      pieces.set(index++, piece);
    }
  }
};

struct Game {
  Game(const Board &board) : header(board) {}
  MarlinFormat header;
  std::vector<std::pair<ViriMove, int16_t>> moves;
};

void datagen(int num_threads, int games);
