#pragma once

#include <ranges>
#include <vector>

constexpr std::vector<std::string_view> string_tokenizer(std::string_view str) {
  return str | std::views::split(' ') |
         std::views::transform(
             [](auto subrange) { return std::string_view(subrange); }) |
         std::ranges::to<std::vector<std::string_view>>();
}

template <typename I, typename C>
constexpr void insertion_sort(I first, I last, C const &&comp) {
  if (first == last)
    return;

  for (auto i = std::next(first); i != last; ++i) {
    auto k = *i;
    auto j = i;
    while (j > first && comp(k, *(j - 1))) {
      *j = *(j - 1);
      --j;
    }
    *j = k;
  }
}
