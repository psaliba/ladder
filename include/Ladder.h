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
#include <boost/asio/buffers_iterator.hpp>
#include "json.hpp"
#include <curl/curl.h>
#include "slacking.hpp"


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
    std::string appToken;
    std::string slackAppID;
    std::string slackClientID;
    std::string slackClientSecret;
    std::string slackChannelName;
    slack::Slacking *slack;
    // The io_context is required for all I/O
    net::io_context ioc;

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12_client};

    // These objects perform our I/O
    tcp::resolver resolver{ioc};

    websocket::stream<beast::ssl_stream<tcp::socket>> ws{ioc, ctx};

public:
    Ladder(std::string &dbPath);

    ~Ladder();

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

    void handleMatchSubmit(json state);
    /**
     * Adds a player to the ladder, at the back of the ladder
     * @param name
     */
    void addPlayer(const std::string &name, const std::string &slackUsername);

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

    /**
     * Event loop for service
     */
    void run();

    // TODO maybe need a updateLadder()

    friend std::ostream &operator<<(std::ostream &os, const Ladder &thisLadder) {
        os << "Ladder: " << std::endl;
        for (const Player &player: thisLadder.ladder) {
            os << player;
        }
        return os;
    }
};