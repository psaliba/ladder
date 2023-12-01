//
// Created by Peter Saliba on 12/1/23.
//
#include <vector>
#include <string>
#include "Player.h"
#include "Match.h"

#ifndef LADDER_LADDER_H
#define LADDER_LADDER_H

#endif //LADDER_LADDER_H

class Ladder {
private:
    std::vector<Player> ladder; // sorted vector containing players to represent the ladder
    std::vector<Match> matchHistory; // a vector containing all match history.
public:
    Ladder();

    /**
     * Adds a player to the ladder, at the back of the ladder
     * @param name
     */
    void addPlayer(const std::string& name);

    /**
     * Updates the passed players rank. Also updates any other player's rank that might need to shift
     * as a result of this move
     * @param name the player to update
     * @param newRank the new rank
     */
    void updateRanking(const std::string& name, int newRank);

    /**
     * Adds the played match into the list of played matches
     * @param matchPlayed
     */
    void recordMatch(Match matchPlayed);

    /**
     * Print all matches recorded to std out
     */
    void printMatches();

    // TODO maybe need a updateLadder()

    friend std::ostream& operator <<(std::ostream& os, const Ladder& thisLadder) {
        os << "Ladder: " << std::endl;
        for (const Player& player : thisLadder.ladder) {
            os << player;
        }
        return os;
    }
};