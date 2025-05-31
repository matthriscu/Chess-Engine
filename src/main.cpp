#include "nnue.hpp"
#include "uciengine.hpp"

int main(int argc, char *argv[]) {
  UCIEngine<NetBoard>(STARTPOS,
                      PerspectiveNetwork(argc > 1 ? argv[1] : NET_PATH))
      .play();
}
