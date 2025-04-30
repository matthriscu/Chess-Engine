#pragma once

#include "perft.hpp"
#include "searcher.hpp"
#include <future>
#include <iostream>
#include <numeric>
#include <string>

class UCIEngine {
  Board position =
      Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  Searcher searcher;
  std::future<void> searcher_future;

private:
  template <typename T> constexpr T parse_number(std::string_view str) const {
    T result = 0;
    std::from_chars<T>(str.begin(), str.end(), result);
    return result;
  }

public:
  void process_command(std::string_view command) {
    std::vector<std::string_view> tokens = string_tokenizer(command);

    if (tokens[0] == "uci")
      std::puts("id name Sah Matt\n"
                "id author Matei Hriscu\n"
                "uciok");
    else if (tokens[0] == "setoption") {
      auto value_it = std::ranges::find(tokens, "value");

      auto accumulator = [](const std::string &acc, std::string_view s) {
        return acc + std::string(s);
      };

      std::string name = std::accumulate(tokens.begin() + 2, value_it,
                                         std::string{}, accumulator);

      if (name == "Hash")
        searcher.resize_ttable(parse_number<size_t>(*(value_it + 1)) *
                               (1 << 20) / sizeof(TTNode));

    } else if (tokens[0] == "isready")
      std::puts("readyok");
    else if (tokens[0].starts_with("position")) {

      size_t moves_list_start;

      if (tokens[1] == "startpos") {
        position =
            Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        moves_list_start = 3;
      } else {
        position =
            Board(tokens | std::views::drop(2) | std::views::take(6) |
                  std::views::join_with(' ') | std::ranges::to<std::string>());
        moves_list_start = 9;
      }

      for (std::string_view move_str :
           std::views::drop(tokens, moves_list_start)) {
        Square from(move_str.substr(0, 2)), to(move_str.substr(2, 2));

        Move move;
        uint16_t mask;

        if (move_str.size() == 5) {
          move = Move(from, to, false, Piece(move_str[4]));
          mask = 0b0011111111111111;
        } else {
          move = Move(from, to, false);
          mask = 0b0000111111111111;
        }

        MoveList moves = position.pseudolegal_moves();

        position.make_move(*std::ranges::find_if(moves, [&](Move m) {
          return (m.raw() & mask) == (move.raw() & mask);
        }));
      }
    } else if (tokens[0] == "go") {
      std::chrono::steady_clock::duration time = std::chrono::years(1);
      long nodes = std::numeric_limits<long>::max(),
           depth = std::numeric_limits<long>::max();

      for (auto [arg, value] :
           tokens | std::views::drop(1) | std::views::adjacent<2>) {
        if (arg == "movetime")
          time = std::chrono::milliseconds{parse_number<long>(value)};
        else if ((position.stm == Sides::WHITE && arg == "wtime") ||
                 (position.stm == Sides::BLACK && arg == "btime"))
          time = std::chrono::milliseconds(parse_number<long>(value) / 20);
        else if ((position.stm == Sides::WHITE && arg == "winc") ||
                 (position.stm == Sides::BLACK && arg == "binc"))
          time += std::chrono::milliseconds(parse_number<long>(value) / 2);
        else if (arg == "nodes")
          nodes = parse_number<long>(value);
        else if (arg == "depth")
          depth = parse_number<long>(value);
      }

      searcher_future =
          std::async(std::launch::async, [this, time, nodes, depth]() {
            std::println("bestmove {}",
                         searcher.search(position, time, nodes, depth).uci());
            std::cout.flush();
          });
    } else if (tokens[0] == "stop")
      searcher.stop();
    else if (tokens[0] == "ucinewgame")
      searcher.clear();
    else if (tokens[0] == "perft")
      std::println("{}", perft(position, parse_number<int>(tokens[1])));
    else if (tokens[0] == "splitperft")
      splitperft(position, parse_number<int>(tokens[1]));
  }

  void play() {
    std::string command;

    while (true) {
      getline(std::cin, command);

      if (command == "quit")
        break;

      process_command(command);
    }
  }
};
