#include "datagen.hpp"
#include "uciengine.hpp"
#include <fstream>
#include <thread>

Game play_datagen_game() {
  Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  int initial_moves = 8 + rand() % 2;

  for (int i = 0; i < initial_moves; ++i) {
    MoveList moves = board.pseudolegal_moves();
    int legal_moves = std::partition(moves.begin(), moves.end(),
                                     [&](const Move &move) {
                                       Board copy = board;
                                       copy.make_move(move);
                                       return copy.is_legal();
                                     }) -
                      moves.begin();

    if (legal_moves == 0) {
      board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
      i = 0;
    } else
      board.make_move(moves[rand() % legal_moves]);
  }

  Searcher searcher;
  Game game(board);

  searcher.add_hash(board.zobrist);

  while (true) {
    auto [move, value] = searcher.search<false>(board, std::chrono::years(1),
                                                5000, 100000, false);

    if (move == Move())
      break;

    game.moves.emplace_back(ViriMove(move),
                            board.stm == Sides::WHITE ? value : -value);
    board.make_move(move);
    searcher.add_hash(board.zobrist);

    if (std::ranges::count(searcher.get_hashes(), board.zobrist) >= 3 ||
        board.is_draw())
      break;
  }

  if (std::ranges::count(searcher.get_hashes(), board.zobrist) >= 3 ||
      board.is_draw() || !board.is_check())
    game.header.wdl = 1;
  else if (board.stm == Sides::WHITE)
    game.header.wdl = 0;
  else
    game.header.wdl = 2;

  return game;
}

void print_game(const Game &game, std::ofstream &out_file) {
  static constexpr int zero = 0;

  static std::mutex mtx;

  std::lock_guard lock(mtx);

  out_file.write(reinterpret_cast<const char *>(&game.header),
                 sizeof(game.header));

  out_file.write(reinterpret_cast<const char *>(game.moves.data()),
                 game.moves.size() * sizeof(game.moves[0]));

  out_file.write(reinterpret_cast<const char *>(&zero), sizeof(zero));
}

void datagen_thread(int games, std::ofstream &out_file) {
  for (int i = 0; i < games; ++i)
    print_game(play_datagen_game(), out_file);
}

void datagen(int num_threads, int games) {
  std::ofstream out_file("datagen.viri", std::ios::binary);
  std::vector<std::jthread> threads;

  for (int i = 0; i < num_threads; ++i)
    threads.emplace_back(datagen_thread, games / num_threads,
                         std::ref(out_file));
}
