#pragma once

#include "tunable_params.hpp"
#include <array>
#include <cstdint>
#include <fstream>
#include <numeric>

struct Accumulator {
  std::array<int16_t, HL> state;

  constexpr Accumulator() : state{} {}

  Accumulator(const std::array<int16_t, HL> &initial_state)
      : state(initial_state) {}

  Accumulator operator+(const Accumulator &other) const {
    Accumulator result = *this;

    std::transform(state.begin(), state.end(), other.state.begin(),
                   result.state.begin(), std::plus{});

    return result;
  }

  Accumulator &operator+=(const Accumulator &other) {
    std::transform(state.begin(), state.end(), other.state.begin(),
                   state.begin(), std::plus{});
    return *this;
  }

  Accumulator operator-(const Accumulator &other) const {
    Accumulator result = *this;

    std::transform(state.begin(), state.end(), other.state.begin(),
                   result.state.begin(), std::minus{});

    return result;
  }

  Accumulator &operator-=(const Accumulator &other) {
    std::transform(state.begin(), state.end(), other.state.begin(),
                   state.begin(), std::minus{});
    return *this;
  }
};

class PerspectiveNetwork {
public:
  std::array<Accumulator, 768> hl_weights;
  Accumulator hl_biases;
  std::array<Accumulator, 2> output_weights;
  int16_t output_bias;

  PerspectiveNetwork(const char *path) {
    std::ifstream in(path, std::ios::binary);
    in.read((char *)this, sizeof(*this));
  }

  const Accumulator &get_hl_line(int index) const { return hl_weights[index]; }

  const Accumulator &get_hl_biases() const { return hl_biases; }

  static constexpr int activation(int16_t x) {
    return std::clamp<int>(x, 0, QA);
  };

  constexpr int compute(const Accumulator &acc_stm,
                        const Accumulator &acc_nstm) const {
    auto compute_hl = [](const Accumulator &acc, const Accumulator &weights) {
      return std::inner_product(
          acc.state.begin(), acc.state.end(), weights.state.begin(), 0,
          std::plus{}, [](int16_t x, int16_t y) { return activation(x) * y; });
    };

    return (compute_hl(acc_stm, output_weights[0]) +
            compute_hl(acc_nstm, output_weights[1]) + output_bias) *
           SCALE / (QA * QB);
  }
};
