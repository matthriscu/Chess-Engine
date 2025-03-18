#include "board.hpp"
#include <iostream>
#include <print>
#include <ranges>
#include <sstream>

void perft(std::string_view fen, size_t depth) {
  auto rec = [&](this auto self, Board &board, size_t depth) {
    if (depth == 0) {
      return 1UZ;
    }

    size_t ans = 0;

    for (Move m :
         std::apply(std::views::take, board.generate_pseudolegal_moves())) {
      Board copy = board;
      copy.make_move(m);

      if (!copy.is_check())
        ans += self(copy, depth - 1);
    }

    return ans;
  };

  Board board(fen);

  for (Move m :
       std::apply(std::views::take, board.generate_pseudolegal_moves())) {
    Board copy = board;
    copy.make_move(m);

    if (!copy.is_check()) {
      if (m.is_promotion())
        std::println("{}{}{} - {} ", to_string(m.from()), to_string(m.to()),
                     piece_to_char(m.promoted_to(), Side::WHITE),
                     rec(copy, depth - 1));
      else
        std::println("{}{} - {}", to_string(m.from()), to_string(m.to()),
                     rec(copy, depth - 1));
    }
  }
}

int main() {
  std::string command;
  Board position;

  while (true) {
    getline(std::cin, command);

    if (command == "uci")
      std::println("id name BasicEngine\n"
                   "id author Matt\n"
                   "uciok");
    else if (command == "isready")
      std::println("readyok");
    else if (command == "ucinewgame")
      ;
    else if (command.starts_with("position")) {
      std::string token;
      std::istringstream iss(command);

      iss >> token >> token;

      if (token == "startpos") {
        position = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w "
                         "- KQkq - 0 1");
        iss >> token;
      } else {
        std::string fen;
        iss >> token;

        do {
          fen += token + ' ';
          iss >> token;
        } while (iss >> token && token != "moves");

        std::println("{}", fen);
        position = Board(fen);
      }

      if (token != "moves")
        iss >> token;

      while (iss >> token) {
        Square from = from_string(token.substr(0, 2)).value(),
               to = from_string(token.substr(2, 2)).value();

        Move move;

        uint16_t mask = 0b0000111111111111;

        if (token.size() == 5) {
          Piece piece = char_to_piece(token[4]).value();
          move = Move(from, to, false, piece);
          mask = 0b0011111111111111;
        } else
          move = Move(from, to, false);

        auto [moves, len] = position.generate_legal_moves();

        position.make_move(
            *std::find_if(moves.begin(), moves.begin() + len, [&](Move m) {
              return (m.raw() & mask) == (move.raw() & mask);
            }));
      }
    } else if (command.starts_with("go")) {
      auto [moves, len] = position.generate_legal_moves();
      std::println("bestmove {}", moves[rand() % len].uci());
    } else if (command.starts_with("quit"))
      exit(0);
  }
}
