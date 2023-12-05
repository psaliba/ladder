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

    std::string getName() const;

    friend std::ostream& operator <<(std::ostream& os, const Player& player) {
        os << player.ranking << ". " << player.name << std::endl;
        return os;
    }
};

//        std::string dbName = "squash_ladder.db";
//        Ladder squashLadder = Ladder(dbName);
//    squashLadder.addPlayer("Eli");
//   squashLadder.addPlayer("Peter");
//    squashLadder.addPlayer("Yoyo");
//    squashLadder.addPlayer("Sam");
//    squashLadder.updateLadder();
//
//        std::cout << squashLadder << std::endl;
//
//        std::time_t t = std::time(nullptr);
//
//        Match match1 = Match("Eli", 3, "Yoyo", 0, *std::localtime(&t));
//        Match match2 = Match("Peter", 3, "Yoyo", 2, *std::localtime(&t));
//        Match match3 = Match("Sam", 3, "Peter", 1, *std::localtime(&t));
//        Match match4 = Match("Peter", 3, "Yoyo", 1, *std::localtime(&t));
//
//        squashLadder.recordMatch(match1);
//        squashLadder.recordMatch(match2);
//        squashLadder.recordMatch(match3);
//        squashLadder.recordMatch(match4);
//
//        std::cout << squashLadder << std::endl;