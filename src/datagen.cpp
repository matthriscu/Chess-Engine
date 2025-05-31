#include "datagen.hpp"

void print_game(const Game &game, std::ofstream &output_file) {
  output_file.write(reinterpret_cast<const char *>(&game.header),
                    sizeof(game.header));

  output_file.write(reinterpret_cast<const char *>(game.moves.data()),
                    game.moves.size() * sizeof(game.moves[0]));

  static constexpr int zero = 0;
  output_file.write(reinterpret_cast<const char *>(&zero), sizeof(zero));
}
