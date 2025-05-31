#pragma once

#include "board.hpp"
#include "searcher.hpp"
#include <filesystem>
#include <fstream>
#include <random>
#include <thread>

template <size_t SIZE> struct NibbleArray {
  std::array<uint8_t, (SIZE + 1) / 2> data;

  constexpr void set(size_t index, uint8_t value) {
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

  constexpr MarlinFormat(const Board &board)
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
  constexpr Game(const Board &board) : header(board) {}
  MarlinFormat header;
  std::vector<std::pair<ViriMove, int16_t>> moves;
};

template <typename BoardType>
  requires std::derived_from<BoardType, Board>
Game play_datagen_game(BoardType board) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist01(0, 1);

  int initial_moves = 8 + dist01(gen);

  for (int i = 0; i < initial_moves; ++i) {
    MoveList moves = board.pseudolegal_moves();
    int legal_moves = std::partition(moves.begin(), moves.end(),
                                     [&](const Move &move) {
                                       BoardType copy = board;
                                       copy.make_move(move);
                                       return copy.is_legal();
                                     }) -
                      moves.begin();

    if (legal_moves == 0)
      return play_datagen_game(board);
    else
      board.make_move(
          moves[std::uniform_int_distribution<>(0, legal_moves - 1)(gen)]);
  }

  Searcher searcher;
  Game game(board);

  searcher.add_hash(board.zobrist);

  while (true) {
    auto [move, value] =
        searcher.search<false>(board, std::nullopt, DATAGEN_SOFT_NODE_LIMIT,
                               DATAGEN_HARD_NODE_LIMIT, std::nullopt);

    if (move == Move())
      break;

    game.moves.emplace_back(ViriMove(move),
                            board.stm == Sides::WHITE ? value : -value);
    board.make_move(move);
    searcher.add_hash(board.zobrist);

    if (searcher.check_threefold(board.zobrist) || board.is_draw())
      break;
  }

  if (searcher.check_threefold(board.zobrist) || board.is_draw() ||
      !board.is_check())
    game.header.wdl = 1;
  else if (board.stm == Sides::WHITE)
    game.header.wdl = 0;
  else
    game.header.wdl = 2;

  return game;
}

void print_game(const Game &game, std::ofstream &output_file);

template <typename BoardType>
  requires std::derived_from<BoardType, Board>
void datagen_thread(int thread_id, const BoardType &initial_board, int games,
                    const std::filesystem::path &output_folder) {
  std::ofstream out_file(output_folder / (std::string("data") +
                                          std::to_string(thread_id) + ".viri"),
                         std::ios::binary);

  for (int i = 0; i < games; ++i)
    print_game(play_datagen_game<BoardType>(initial_board), out_file);
}

template <typename BoardType>
  requires std::derived_from<BoardType, Board>
void datagen(int num_threads, int games, const BoardType &initial_board,
             const std::filesystem::path &output_folder) {

  std::vector<std::jthread> threads;

  std::filesystem::create_directory(output_folder);

  for (int i = 0; i < num_threads; ++i)
    threads.emplace_back(datagen_thread<BoardType>, i, initial_board,
                         games / num_threads + (i < games % num_threads),
                         output_folder);
}
