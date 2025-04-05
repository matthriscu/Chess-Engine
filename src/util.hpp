#pragma once

#include <ranges>
#include <vector>

constexpr std::vector<std::string_view> string_tokenizer(std::string_view str) {
  return str | std::views::split(' ') |
         std::views::transform(
             [](auto subrange) { return std::string_view(subrange); }) |
         std::ranges::to<std::vector<std::string_view>>();
}
