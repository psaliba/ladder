#include <iostream>
#include "include/Ladder.h"



int main() {
    std::string dbName = "squash_ladder.db";
    Ladder squashLadder = Ladder(dbName);
    squashLadder.addPlayer("Eli");
    squashLadder.addPlayer("Peter");
    squashLadder.addPlayer("Yoyo");
    squashLadder.addPlayer("Sam");

    std::cout << squashLadder << std::endl;

    std::time_t t = std::time(nullptr);

    Match match1 = Match("Eli", 3, "Yoyo", 0, *std::localtime(&t));
    Match match2 = Match("Peter", 3, "Yoyo", 2, *std::localtime(&t));
    Match match3 = Match("Sam", 3, "Peter", 1, *std::localtime(&t));
    Match match4 = Match("Peter", 3, "Yoyo", 1, *std::localtime(&t));

    squashLadder.recordMatch(match1);
    squashLadder.recordMatch(match2);
    squashLadder.recordMatch(match3);
    squashLadder.recordMatch(match4);

    squashLadder.saveMatchToDatabase(match1);

    squashLadder.printMatches();
    std::cout << squashLadder << std::endl;

    return 0;
}

// TODO get sqlite running. table for ladder, table for match history
// might need a remove player. the recordMatch function queries for the the most recent
// ladder in the db. updates it if needed. adds match to db of matches