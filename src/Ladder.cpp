//
// Created by Peter Saliba on 12/1/23.
//

#include "../include/Ladder.h"
#include <sqlite3.h>
#include <iostream>
#include <fstream>


Ladder::Ladder(std::string &dbPath) {

    int rc = sqlite3_open(dbPath.c_str(), &db);

    if (rc != 0) {
        std::cerr << "Couldn't open the db " << sqlite3_errmsg(db) << std::endl;
        exit(1);
    }
    std::cout << "Opened the db successfully" << std::endl;
    // TODO each time we update the ladder, need to also update the db

    createTables();
    loadLadder();

    // set up slack connection.
    // expects a file called secret.txt containing the following (in order, each token on a new line)
    // slack xapp-1 token for socket connection
    // slack app id
    // slack client id
    // slack bot user oAuth token
    // the name of the channel to post to
    std::string doNotUse;
    std::ifstream infile("secret.txt");
    std::getline(infile, doNotUse); // skip first
    std::getline(infile, slackAppID);
    std::getline(infile, slackClientID);
    std::getline(infile, slackClientSecret);
    std::getline(infile, slackChannelName);

    auto& slack = slack::create(slackClientSecret);
    slack.chat.channel = slackChannelName;

    slack.chat.postMessage("this is a test");
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
                                             "date TEXT NOT NULL,"
                                             "score TEXT NOT NULL);";

    executeSQL(createLadderTableSQL);
    executeSQL(createMatchHistoryTableSQL);
}

void Ladder::loadLadder() {
    const char* ladderQuery =  "SELECT * FROM ladder ORDER BY ranking;";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, ladderQuery, -1, &stmt, nullptr);

    if (rc == 0) {
        // Clear existing ladder data
        ladder.clear();

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            int ranking = sqlite3_column_int(stmt, 2);
            ladder.emplace_back(name, ranking);
        }

        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to execute query: " << sqlite3_errmsg(db) << std::endl;
        exit(1);
    }
}


void Ladder::saveMatchToDatabase(Match &match) {
    std::string insertMatchSQL = "INSERT INTO match_history (winner, loser, date, score) VALUES ('" + match.getWinnerName() + "', '" + match.getLooserName() + "', '" + match.getStringDate() + + "', '" + match.getStringScore() + "');";
    executeSQL(insertMatchSQL.c_str());
}

void Ladder::addPlayer(const std::string &name) {
    ladder.emplace_back(name, ladder.size() + 1);
    std::string insertMatchSQL = "INSERT INTO ladder (name, ranking) VALUES ('" + name + "', '" + std::to_string(ladder.size()+1) + "');";
    executeSQL(insertMatchSQL.c_str());
}

void Ladder::updateLadder() {
    for (const Player& player : ladder) {
        std::string updateSQL = "UPDATE ladder SET ranking = " + std::to_string(player.getRanking()) + " WHERE name = '" + player.getName() + "';";
        executeSQL(updateSQL.c_str());
    }
}

void Ladder::recordMatch(Match matchPlayed) {
    matchHistory.emplace_back(matchPlayed);
    saveMatchToDatabase(matchPlayed);

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
    updateLadder(); // not the most efficient to do it here, but makes sure the this.ladder matches the db
}

void Ladder::printMatches() {
    std::cout << "Matches recorded:" << std::endl;
    for (const Match& match : matchHistory) {
        std::cout << match;
    }
}
