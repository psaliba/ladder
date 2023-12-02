//
// Created by Peter Saliba on 12/1/23.
//

#include "../include/Ladder.h"
#include <sqlite3.h>
#include <iostream>



Ladder::Ladder(std::string &dbPath) {

    int rc = sqlite3_open(dbPath.c_str(), &db);

    if (rc != 0) {
        std::cerr << "Couldn't open the db " << sqlite3_errmsg(db) << std::endl;
        exit(1);
    }
    std::cout << "Opened the db successfully" << std::endl;
    // TODO load ladder/matches from DB
    // TODO each time we update the ladder, need to also update the db 

    createTables();
}

void Ladder::executeSQL(const char *sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);

    if (rc != 0) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        exit(1);
    }
}

void Ladder::createTables() {
    const char* createLadderTableSQL = "CREATE TABLE IF NOT EXISTS ladder ("
                                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                       "name TEXT NOT NULL,"
                                       "ranking INTEGER NOT NULL);";

    const char* createMatchHistoryTableSQL = "CREATE TABLE IF NOT EXISTS match_history ("
                                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                             "winner TEXT NOT NULL,"
                                             "loser TEXT NOT NULL,"
                                             "date TEXT NOT NULL);";

    executeSQL(createLadderTableSQL);
    executeSQL(createMatchHistoryTableSQL);
}

void Ladder::saveMatchToDatabase(Match &match) {
    std::string insertMatchSQL = "INSERT INTO match_history (winner, loser, date) VALUES ('" + match.getWinnerName() + "', '" + match.getLooserName() + "', '" + match.getStringDate() + "');";
    executeSQL(insertMatchSQL.c_str());
}

void Ladder::addPlayer(const std::string &name) {
    ladder.emplace_back(name, ladder.size() + 1);
}

void Ladder::recordMatch(Match matchPlayed) {
    matchHistory.emplace_back(matchPlayed);

    auto winnerIt = std::find_if(ladder.begin(), ladder.end(),
                                 [&matchPlayed](const Player& player) {
                                     return player.getName() == matchPlayed.getWinnerName();
                                 });

    auto loserIt = std::find_if(ladder.begin(), ladder.end(),
                                [&matchPlayed](const Player& player) {
                                    return player.getName() == matchPlayed.getLooserName();
                                });

    if (winnerIt != ladder.end() && loserIt != ladder.end()) {
        // Swap ladder spots if the winner has a lower rank
        if (winnerIt->getRanking() > loserIt->getRanking()) {
            int newRank = winnerIt->getRanking();
            winnerIt->setRanking(loserIt->getRanking());
            loserIt->setRanking(newRank);
        }
    }
    std::sort(ladder.begin(), ladder.end(), [](const Player& a, const Player& b) {
        return a.getRanking() < b.getRanking();
    });

}

void Ladder::printMatches() {
    std::cout << "Matches recorded:" << std::endl;
    for (const Match& match : matchHistory) {
        std::cout << match;
    }
}
