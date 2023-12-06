//
// Created by Peter Saliba on 12/1/23.
//

#include "../include/Ladder.h"
#include <sqlite3.h>
#include <iostream>
#include <fstream>


Ladder::Ladder(std::string &dbPath) {
    // set up slack connection.
    // expects a file called secret.txt containing the following (in order, each token on a new line)
    // slack xapp-1 token for socket connection
    // slack app id
    // slack client id
    // slack bot user oAuth token
    // the name of the channel to post to
    std::string appToken;
    std::ifstream infile("secret.txt");
    std::getline(infile, appToken); // skip first
    std::getline(infile, slackAppID);
    std::getline(infile, slackClientID);
    std::getline(infile, slackClientSecret);
    std::getline(infile, slackChannelName);
    createSlackConnection(appToken);

    beast::flat_buffer buffer;
    wss->read(buffer);
    std::cout << beast::make_printable(buffer.data()) << std::endl;
    wss->close(websocket::close_code::normal);

    int rc = sqlite3_open(dbPath.c_str(), &db);

    if (rc != 0) {
        std::cerr << "Couldn't open the db " << sqlite3_errmsg(db) << std::endl;
        exit(1);
    }
    std::cout << "Opened the db successfully" << std::endl;
    // TODO each time we update the ladder, need to also update the db
    createTables();
    loadLadder();



//    auto& slack = slack::create(slackClientSecret);
//    slack.chat.channel = slackChannelName;
//
//    slack.chat.postMessage("this is a test");
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

void Ladder::createSlackConnection(std::string appToken) {
    const std::string endOfURI = performSocketCurlCheck(appToken);
    std::string host = "wss-primary.slack.com";
    const char *port = "443";
    const char *text = endOfURI.c_str();
    try {
        // The io_context is required for all I/O
        net::io_context ioc;

        // The SSL context is required, and holds certificates
        ssl::context ctx{ssl::context::tlsv12_client};

        // These objects perform our I/O
        tcp::resolver resolver{ioc};

       websocket::stream<beast::ssl_stream<tcp::socket>> ws{ioc, ctx};

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        auto ep = net::connect(get_lowest_layer(ws), results);

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.c_str()))
            throw beast::system_error(
                    beast::error_code(
                            static_cast<int>(::ERR_get_error()),
                            net::error::get_ssl_category()),
                    "Failed to set SNI Hostname");

        // Update the host_ string. This will provide the value of the
        // Host HTTP header during the WebSocket handshake.
        // See https://tools.ietf.org/html/rfc7230#section-5.4
        host += ':' + std::to_string(ep.port());

        // Perform the SSL handshake
        ws.next_layer().handshake(ssl::stream_base::client);

        // Set a decorator to change the User-Agent of the handshake
        ws.set_option(websocket::stream_base::decorator(
                [](websocket::request_type &req) {
                    req.set(http::field::user_agent,
                            std::string(BOOST_BEAST_VERSION_STRING) +
                            " websocket-client-coro");
                }));

        // Perform the websocket handshake
        ws.handshake(host, text);
        wss = &ws;
    }
    catch(std::exception const & e) {
        std::cerr << "Error: " << e.what() << std::endl;
        exit(1);
    }
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *output) {
    size_t total_size = size * nmemb;
    output->append((char *) contents, total_size);
    return total_size;
}

std::string Ladder::performSocketCurlCheck(const std::string& token) {
    CURL *curl;
    CURLcode res;

    // Initialize cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    std::string response;

    // Set the URL for the POST request
    curl_easy_setopt(curl, CURLOPT_URL, "https://slack.com/api/apps.connections.open");

    // Set the POST data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

    // Set the custom headers
    struct curl_slist *headers = nullptr;
    std::string tokenHeader = "Authorization: Bearer " + token;
    headers = curl_slist_append(headers, "Content-type: application/x-www-form-urlencoded");
    headers = curl_slist_append(headers, tokenHeader.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Set the callback function to handle the response
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Perform the POST request
    res = curl_easy_perform(curl);

    // Check for errors
    if (res != CURLE_OK) {
        fprintf(stderr, "slack socket url request failed: %s\n", curl_easy_strerror(res));
        exit(1);
    }

    // Clean up
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    json parsedResponse = json::parse(response);
    std::string parsedHost = parsedResponse["url"];

    size_t lastSlashPos = parsedHost.find_last_of('/');
    std::string endOfURI = "/link" + parsedHost.substr(lastSlashPos);
    return endOfURI;
}
