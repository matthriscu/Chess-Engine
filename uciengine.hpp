#pragma once

#include "board.hpp"
#include "perft.hpp"
#include "searcher.hpp"
#include <iostream>

class UCIEngine {
  Board position;
  Searcher searcher;

private:
  constexpr int stoi(std::string_view str) const {
    int result;
    std::from_chars(str.begin(), str.end(), result);
    return result;
  }

public:
  void process_command(std::string_view command) {
    if (command == "uci")
      std::puts("id name <name>\n"
                "id author <author>\n"
                "uciok");
    else if (command == "isready")
      std::puts("readyok");
    else if (command.starts_with("position")) {
      std::vector<std::string_view> tokens = string_tokenizer(command);

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
    } else if (command.starts_with("go")) {
      std::vector<std::string_view> tokens = string_tokenizer(command);
      bool movetime = tokens[1] == "movetime";

      int relevant_time_index =
              (movetime || position.stm == Sides::WHITE) ? 2 : 4,
          time = stoi(tokens[relevant_time_index]),
          relevant_increment_index = relevant_time_index + 4;

      if (!movetime)
        time /= 20;

      if (relevant_increment_index < tokens.size())
        time += stoi(tokens[relevant_increment_index]) / 2;

      std::println("bestmove {}", searcher.search(position, time).uci());
    } else if (command.starts_with("perft")) {
      std::vector<std::string_view> tokens = string_tokenizer(command);

      std::println("{}", perft(position, stoi(tokens[1])));
    } else if (command.starts_with("splitperft")) {
      std::vector<std::string_view> tokens = string_tokenizer(command);

      splitperft(position, stoi(tokens[1]));
    }
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
