//
// Created by Peter Saliba on 12/1/23.
//
#include <vector>
#include <string>
#include <sqlite3.h>
#include "Player.h"
#include "Match.h"
#include "socket.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include "json.hpp"
#include <curl/curl.h>


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using json = nlohmann::json;

#ifndef LADDER_LADDER_H
#define LADDER_LADDER_H

#endif //LADDER_LADDER_H

class Ladder {
private:
    std::vector<Player> ladder; // sorted vector containing players to represent the ladder
    std::vector<Match> matchHistory; // a vector containing all match history.
    sqlite3 *db{};
    std::string slackAppID;
    std::string slackClientID;
    std::string slackClientSecret;
    std::string slackChannelName;
    websocket::stream<beast::ssl_stream<tcp::socket>> *wss;

public:
    Ladder(std::string &dbPath);

    /**
     * Executes the passed sql statement on the db
     * @param sql
     */
    void executeSQL(const char *sql);

    /**
     * Make the db tables if they do not exist already
     */
    void createTables();

    /**
     * Inserts the passed match into the db
     * @param match the match to save
     */
    void saveMatchToDatabase(Match &match);

    /**
     * Updates the entire ladder in the database. Sets all players in the db to have their ranking as defined by this.ladder
     */
    void updateLadder();

    /**
     * Load the ladder from the attached database, replacing any data that exists in the ladder vector
     */
    void loadLadder();

    /**
     * Adds a player to the ladder, at the back of the ladder
     * @param name
     */
    void addPlayer(const std::string &name);

    /**
     * Updates the passed players rank. Also updates any other player's rank that might need to shift
     * as a result of this move
     * @param name the player to update
     * @param newRank the new rank
     */
    void updateRanking(const std::string &name, int newRank);

    /**
     * Adds the played match into the list of played matches
     * @param matchPlayed
     */
    void recordMatch(Match matchPlayed);

    /**
     * Print all matches recorded to std out
     */
    void printMatches();

    void createSlackConnection(std::string basicString);

    static std::string performSocketCurlCheck(const std::string& token);

    // TODO maybe need a updateLadder()

    friend std::ostream &operator<<(std::ostream &os, const Ladder &thisLadder) {
        os << "Ladder: " << std::endl;
        for (const Player &player: thisLadder.ladder) {
            os << player;
        }
        return os;
    }
};