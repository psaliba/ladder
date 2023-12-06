#include "include/Ladder.h"
#include <string>



//------------------------------------------------------------------------------


int main() {

    std::string dbName = "squash_ladder.db";
    Ladder squashLadder = Ladder(dbName);
    squashLadder.addPlayer("Eli Paul", "U02F0JHJD6J");
    squashLadder.addPlayer("Jacob Domber", "U02FLCCFT8V");
    squashLadder.addPlayer("Ryan Lake", "U02FDU3U79V");
    squashLadder.addPlayer("Sam Cotsarelis", "U042MF8M9HD");
    squashLadder.addPlayer("Krishnav Kishorepuria", "U05TX7QP1K7");
    squashLadder.addPlayer("Peter Saliba", "U02GD2Z6VC0");
    squashLadder.addPlayer("Yoyo Tobunluepop", "U02F4AUBFRB");
    squashLadder.addPlayer("Arjan Trehan", "U05U6TF5QTG");
    squashLadder.addPlayer("Matthew Giuliano", "U02JB30P71U");
    squashLadder.addPlayer("Harrison Seeley", "U02F45RMEBY");
    squashLadder.addPlayer("Daniel Yanni", "U0542QK9P3M");
    squashLadder.addPlayer("Vanel Joseph", "U02F4GZGQ4D");
    squashLadder.addPlayer("Charles Egan", "U05U8MVCYEM");
    squashLadder.updateLadder();

    squashLadder.run();
}
