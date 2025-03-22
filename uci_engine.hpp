#pragma once

#include "board.hpp"
#include "common.hpp"
#include <optional>
#include <ranges>
#include <string>

class UCIEngine {
  static constexpr std::string name = "name", author = "author";
  Board position;

  constexpr std::optional<std::string>
  process_command(std::string_view command) {
    if (command == "uci")
      return format("id name {}\n"
                    "id author {}\n"
                    "uciok",
                    name, author);
    else if (command == "isready")
      return "readyok";
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
          move = Move(from, to, false, char_to_piece(move_str[4]).value());
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

      return std::nullopt;
    } else if (command.starts_with("go")) {
      std::vector<std::string_view> tokens = string_tokenizer(command);
      size_t relevant_time_index = position.stm == Side::WHITE ? 2 : 4;

      size_t time = 0;
      std::from_chars(tokens[relevant_time_index].begin(),
                      tokens[relevant_time_index].end(), time);

      return format("bestmove {}",
                    position
                        .bestmove(std::chrono::steady_clock::now() +
                                  std::chrono::milliseconds(time / 30))
                        .uci());
    } else
      return std::nullopt;
  }

public:
  void play();
};
