#include "uci_engine.hpp"
#include <iostream>

void UCIEngine::play() {
  std::string command;

  while (true) {
    getline(std::cin, command);

    if (command == "quit")
      break;

    std::optional<std::string> reply = process_command(command);

    if (reply)
      std::println("{}", reply.value());
  }
}
