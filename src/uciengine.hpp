#pragma once

#include "datagen.hpp"
#include "perft.hpp"
#include "searcher.hpp"
#include <future>
#include <iostream>
#include <string>

template <typename T> constexpr T parse_number(std::string_view str) {
  T result = 0;
  std::from_chars<T>(str.begin(), str.end(), result);
  return result;
}

template <typename BoardType>
  requires std::derived_from<BoardType, Board>
class UCIEngine {
  Searcher searcher;
  std::future<void> searcher_future;
  BoardType position;

public:
  template <typename... Args>
  UCIEngine(Args &&...args) : position(std::forward<Args>(args)...) {}

  void process_command(std::string_view command) {
    using std::literals::string_literals::operator""s;
    std::vector<std::string_view> tokens = string_tokenizer(command);

    if (tokens[0] == "uci")
      std::puts("id name Sah Matt\n"
                "id author Matei Hriscu\n"
                "option name Hash type spin default 64 min 1 max 16384\n"
                "uciok");
    else if (tokens[0] == "setoption") {
      auto value_it = std::ranges::find(tokens, "value");

      std::string name = join_tokens(std::span{tokens.begin() + 2, value_it}),
                  value = join_tokens(std::span{value_it + 1, tokens.end()});

      if (name == "Hash")
        searcher.resize_ttable(parse_number<size_t>(value) * (1 << 20) /
                               sizeof(TTNode));
    } else if (tokens[0] == "isready")
      std::puts("readyok");
    else if (tokens[0].starts_with("position")) {
      position = NetBoard(
          tokens[1] == "startpos"
              ? std::string(STARTPOS)
              : join_tokens(tokens | std::views::drop(2) | std::views::take(6),
                            ' '),
          position.net);

      searcher.clear_hashes();
      searcher.add_hash(position.zobrist);

      for (std::string_view move_str :
           std::views::drop_while(tokens, [](std::string_view token) {
             return token != "moves";
           }) | std::views::drop(1)) {
        position.make_move(position.pseudolegal_moves().get_matching_move(
            move_str.substr(0, 2), move_str.substr(2, 2),
            move_str.size() == 5 ? Piece(move_str.back()) : Piece()));
        searcher.add_hash(position.zobrist);
      }
    } else if (tokens[0] == "go") {
      std::optional<std::chrono::steady_clock::duration> duration;
      std::optional<int64_t> nodes, depth;

      for (auto [arg, value] :
           tokens | std::views::drop(1) | std::views::adjacent<2>) {
        if (arg == "movetime")
          duration = std::chrono::milliseconds{parse_number<int64_t>(value)};
        else if ((position.stm == Sides::WHITE && arg == "wtime") ||
                 (position.stm == Sides::BLACK && arg == "btime"))
          duration =
              std::chrono::milliseconds(parse_number<int64_t>(value) / 20);
        else if ((position.stm == Sides::WHITE && arg == "winc") ||
                 (position.stm == Sides::BLACK && arg == "binc"))
          *duration +=
              std::chrono::milliseconds(parse_number<int64_t>(value) / 2);
        else if (arg == "nodes")
          nodes = parse_number<int64_t>(value);
        else if (arg == "depth")
          depth = parse_number<int64_t>(value);
      }

      searcher_future = std::async(std::launch::async, [this, duration, nodes,
                                                        depth]() {
        std::println("bestmove {}",
                     searcher.search(position, duration, nodes, nodes, depth)
                         .first.uci());
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
    else if (tokens[0] == "print")
      std::println("{}", static_cast<Board>(position));
    else if (tokens[0] == "datagen")
      datagen(parse_number<int>(tokens[1]), parse_number<int>(tokens[2]),
              position, tokens[3]);
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
