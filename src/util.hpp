#pragma once

#include <ranges>
#include <vector>

inline std::string_view STARTPOS =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

constexpr std::vector<std::string_view> string_tokenizer(std::string_view str) {
  return str | std::views::split(' ') |
         std::views::transform(
             [](auto subrange) { return std::string_view(subrange); }) |
         std::ranges::to<std::vector<std::string_view>>();
}

constexpr std::string join_tokens(auto strings, char delimiter = ' ') {
  return strings | std::views::join_with(delimiter) |
         std::ranges::to<std::string>();
}
