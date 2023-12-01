//
// Created by Peter Saliba on 12/1/23.
//
#include <string>
#include <iostream>
#include <ctime>
#ifndef LADDER_MATCH_H
#define LADDER_MATCH_H

#endif //LADDER_MATCH_H

// A squash match played for a ladder spot. Matches are best of 5, games to 11.
class Match {
private:
    std::string winner;
    std::string looser;
    int winnerGameScore; // the number of games won by the winner. will almost always be 3, but could be less because of opposing player's retirement.
    int looserGameScore; // the number of games won by the looser. will always be less than 3
    std::tm datePlayed;
public:
    Match(std::string winner, int winnerGameScore, std::string looser, int looserGameScore, std::tm dateOfMatch);

    std::string getWinnerName() const;
    std::string getLooserName() const;

    friend std::ostream& operator <<(std::ostream& os, const Match& match) {
        os << "Match: " << match.winner << " beat " << match.looser << " " << match.winnerGameScore << "-" << match.looserGameScore << std::endl;
        return os;
    }
};

