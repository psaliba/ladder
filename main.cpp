#include <iostream>
#include "include/Ladder.h"



int main() {
    std::string dbName = "squash_ladder.db";
    Ladder squashLadder = Ladder(dbName);
//    squashLadder.addPlayer("Eli");
//    squashLadder.addPlayer("Peter");
//    squashLadder.addPlayer("Yoyo");
//    squashLadder.addPlayer("Sam");
//    squashLadder.updateLadder();

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

    std::cout << squashLadder << std::endl;

    return 0;
}
