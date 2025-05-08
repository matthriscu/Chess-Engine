#include "datagen.hpp"
#include "uciengine.hpp"

int main(int argc, char *argv[]) {
  if (argc >= 2) {
    if (std::string(argv[1]) == "datagen")
      datagen(parse_number<int>(argv[2]), parse_number<int>(argv[3]));
  } else
    UCIEngine().play();
}
