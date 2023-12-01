//
// Created by Peter Saliba on 12/1/23.
//

#include "../include/Ladder.h"

Ladder::Ladder() = default;

void Ladder::addPlayer(const std::string &name) {
    ladder.emplace_back(name, ladder.size() + 1);
    std::cout << "added " << name << " to the ladder";
}

