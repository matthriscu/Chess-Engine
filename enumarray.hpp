#pragma once

#include <array>

template <typename E, class T, std::size_t N>
struct EnumArray : public std::array<T, N> {
  constexpr T &operator[](E e) {
    return std::array<T, N>::operator[]((std::size_t)e);
  }

  constexpr const T &operator[](E e) const {
    return std::array<T, N>::operator[]((std::size_t)e);
  }
};
