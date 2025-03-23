#pragma once

#include "board.hpp"
#include "perft.hpp"
#include <iostream>

class UCIEngine {
  Board position;

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
      size_t relevant_time_index = position.stm == Sides::WHITE ? 2 : 4;

      size_t time = 0;
      std::from_chars(tokens[relevant_time_index].begin(),
                      tokens[relevant_time_index].end(), time);

      std::println("bestmove {}",
                   position
                       .bestmove(std::chrono::steady_clock::now() +
                                 std::chrono::milliseconds(time / 30))
                       .uci());
    } else if (command.starts_with("perft")) {
      std::vector<std::string_view> tokens = string_tokenizer(command);

      int depth;
      std::from_chars(tokens[1].begin(), tokens[1].end(), depth);

      std::println("{}", perft(position, depth));
    } else if (command.starts_with("splitperft")) {
      std::vector<std::string_view> tokens = string_tokenizer(command);

      int depth;
      std::from_chars(tokens[1].begin(), tokens[1].end(), depth);

      splitperft(position, depth);
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
