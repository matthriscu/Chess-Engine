#include "board.hpp"
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
      std::vector<std::string_view> tokens =
          command | std::views::split(' ') |
          std::views::transform(
              [](auto subrange) { return std::string_view(subrange); }) |
          std::ranges::to<std::vector<std::string_view>>();

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
           std::ranges::drop_view(tokens, moves_list_start)) {
        Square from = from_string(move_str.substr(0, 2)).value(),
               to = from_string(move_str.substr(2, 2)).value();

        Move move;
        uint16_t mask;

        if (move_str.size() == 5) {
          move = Move(from, to, false, char_to_piece(move_str[4]).value());
          mask = 0b0011111111111111;
        } else {
          move = Move(from, to, false);
          mask = 0b0000111111111111;
        }

        auto [moves, len] = position.generate_legal_moves();

        position.make_move(
            *std::find_if(moves.begin(), moves.begin() + len, [&](Move m) {
              return (m.raw() & mask) == (move.raw() & mask);
            }));
      }

      return std::nullopt;
    } else if (command.starts_with("go")) {
      auto [moves, len] = position.generate_legal_moves();
      return format("bestmove {}", moves[rand() % len].uci());
    } else
      return std::nullopt;
  }

public:
  void play();
};
