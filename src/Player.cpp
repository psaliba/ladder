//
// Created by Peter Saliba on 12/1/23.
//

#include <utility>
#include <iostream>

#include "../include/Player.h"

Player::Player(std::string playerName, int ladderRanking) {
    ranking = ladderRanking;
    name = std::move(playerName); //avoids copying multiple times
}

void Player::setRanking(int newRank) {
    std::cout << "setting player " << name << " to have new rank " << newRank << std::endl;
    ranking = newRank;
}

int Player::getRanking() const {
    return ranking;
}

std::string Player::getName() const {
    return name;
}




