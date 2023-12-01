#include <iostream>
#include "include/Ladder.h"


int main() {
    Ladder squashLadder = Ladder();
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


    squashLadder.printMatches();
    std::cout << squashLadder << std::endl;

    return 0;
}
