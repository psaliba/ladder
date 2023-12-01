//
// Created by Peter Saliba on 12/1/23.
//
#include <iostream>
#include <string>

#ifndef LADDER_PLAYER_H
#define LADDER_PLAYER_H

#endif //LADDER_PLAYER_H


class Player {
private:
    int  ranking;
    std::string name;

public:
    Player(std::string playerName, int ladderRanking);

    /**
     * Sets the rank of this player to the passed value
     * @param newRank
     */
    void setRanking(int newRank);

    /**
     * Get this players ranking
     * @return the rank of this player
     */
    int getRanking() const;

    friend std::ostream& operator <<(std::ostream& os, const Player& player) {
        os << player.ranking << ". " << player.name << std::endl;
        return os;
    }
};