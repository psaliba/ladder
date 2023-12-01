//
// Created by Peter Saliba on 12/1/23.
//

#include "../include/Match.h"
#include <string>
#include <utility>

Match::Match(std::string winnerName, int winningGames, std::string looserName, int loosingGames, std::tm dateOfMatch) {
    winner = std::move(winnerName);
    looser = std::move(looserName);
    winnerGameScore = winningGames;
    looserGameScore = loosingGames;
    datePlayed = dateOfMatch;
}

std::string Match::getWinnerName() const {
    return winner;
}

std::string Match::getLooserName() const {
    return looser;
}