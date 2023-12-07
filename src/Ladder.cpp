//
// Created by Peter Saliba on 12/1/23.
//

#include "../include/Ladder.h"
#include <sqlite3.h>
#include <iostream>
#include <fstream>


Ladder::Ladder(std::string &dbPath) {
    // set up slack connection.
    std::ifstream infile("secret.txt");
    std::getline(infile, appToken);
    std::getline(infile, slackAppID);
    std::getline(infile, slackClientID);
    std::getline(infile, slackClientSecret);
    std::getline(infile, slackChannelName);
    createSlackConnection();

    //needed to post a message without interaction
    slack = &slack::create(slackClientSecret);

    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != 0) {
        std::cerr << "Couldn't open the db " << sqlite3_errmsg(db) << std::endl;
        exit(1);
    }
    std::cout << "Opened the db successfully" << std::endl;
    createTables();
    loadLadder();
}

Ladder::~Ladder() {
    ws.close(websocket::close_code::normal);
    sqlite3_close(db);
}

void Ladder::run() {
    beast::flat_buffer buffer;
    while (true) {
        // read call is blocking, so will only run when we get a new message from the socket
        ws.read(buffer);
        std::cout << beast::make_printable(buffer.data()) << std::endl;
        ws.text(false);
        const char *data = boost::asio::buffer_cast<const char *>(buffer.data());
        std::size_t size = buffer.size();
        std::string reqString(data, size);
        auto request = json::parse(reqString);

        if (request["type"] == "disconnect") {
            // we want to refresh connection, so close socket and retry
            std::cout << "Heard message with type `disconnect`, refreshing socket..." << std::endl;
            break;
        } else if (request["payload"]["type"] == "block_actions") {
            handleBlockAction(request);
        } else if (request["type"] == "slash_commands") {
            handleSlashCommand(request);
        }
        buffer.consume(buffer.size()); // clears buffer
    }
    ws.close(websocket::close_code::normal);
    createSlackConnection();
    run();
}

void Ladder::handleBlockAction(json request) {
    std::stringstream ss;
    std::string evID = request["envelope_id"];
    json res = json::parse(R"(
              {
                "envelope_id": 3.141,
                "payload": {}
              }
            )");
    res["envelope_id"] = evID;

    // check to see if submit button was pressed
    if (request["payload"]["actions"][0]["action_id"] == "button-action") {
        std::cout << "heard submit... " << std::endl;
        slackChannelName = request["payload"]["channel"]["id"];
        handleMatchSubmit(request["payload"]["state"]);
    }
    std::cout << res.dump() << std::endl;
    ws.text(true);
    ws.write(net::buffer(res.dump()));
}

void Ladder::handleSlashCommand(json request) {
    std::stringstream ss;
    ss << *this;
    std::string ladderString = ss.str();
    std::string evID = request["envelope_id"];
    json res = json::parse(R"(
              {
                "envelope_id": "to be replaced",
                "payload": {}
              }
            )");
    res["envelope_id"] = evID;
    std::string toWrite;
    if (request["payload"]["command"] == "/match") {
        std::cout << "heard /match command" << std::endl;
        res["payload"] = json::parse(R"({
	"blocks": [
		{
			"type": "section",
			"text": {
				"type": "mrkdwn",
				"text": "Winner"
			},
			"accessory": {
				"type": "users_select",
				"placeholder": {
					"type": "plain_text",
					"text": "Select a user",
					"emoji": true
				},
				"action_id": "users_select-winner"
			}
		},
		{
			"type": "section",
			"text": {
				"type": "mrkdwn",
				"text": "Looser"
			},
			"accessory": {
				"type": "users_select",
				"placeholder": {
					"type": "plain_text",
					"text": "Select a user",
					"emoji": true
				},
				"action_id": "users_select-looser"
			}
		},
		{
			"type": "section",
			"text": {
				"type": "mrkdwn",
				"text": "Score:"
			},
			"accessory": {
				"type": "radio_buttons",
				"options": [
					{
						"text": {
							"type": "plain_text",
							"text": "3-0",
							"emoji": true
						},
						"value": "0"
					},
					{
						"text": {
							"type": "plain_text",
							"text": "3-1",
							"emoji": true
						},
						"value": "1"
					},
					{
						"text": {
							"type": "plain_text",
							"text": "3-2",
							"emoji": true
						},
						"value": "2"
					}
				],
				"action_id": "radio_buttons-action"
			}
		},
		{
			"type": "section",
			"text": {
				"type": "mrkdwn",
				"text": "Click 'Submit' to enter the challenge match score"
			},
			"accessory": {
				"type": "button",
				"text": {
					"type": "plain_text",
					"text": "Submit",
					"emoji": true
				},
				"value": "true",
				"action_id": "button-action"
			}
		}
	]
})");
        toWrite = res.dump();
    } else if (request["payload"]["command"] == "/ladder") {
        res["payload"] = json::parse(R"({
                    "response_type": "in_channel",
                    "blocks": [
                    {
                        "type": "section",
                        "text": {
                            "type": "plain_text",
                            "text": "",
                            "emoji": true
                            }
                    }
                    ]
            })");
        res["payload"]["blocks"][0]["text"]["text"] = ladderString;
        std::cout << ladderString << std::endl;
        toWrite = res.dump();
    }
    std::cout << "Heard slash command, responding with " << toWrite << std::endl;
    ws.text(true);
    ws.write(net::buffer(toWrite));
}

void Ladder::handleMatchSubmit(json state) {
    std::string winnerID;
    std::string looserID;
    int looserScore;
    std::string toSend = "t";
    if (state["values"].size() < 3) {
        toSend = "Error: Please make sure to enter all fields before clicking 'Submit'";
    }

    for (auto &values: state["values"].items()) {
        std::cout << values.value() << std::endl;
        if (values.value().find("users_select-winner") != values.value().end()) {
            winnerID = values.value()["users_select-winner"]["selected_user"].dump();
        } else if (values.value().find("users_select-looser") != values.value().end()) {
            looserID = values.value()["users_select-looser"]["selected_user"].dump();
        } else if (values.value().find("radio_buttons-action") != values.value().end()) {
            std::string score = values.value()["radio_buttons-action"]["selected_option"]["value"];
            looserScore = std::stoi(score);
        }
    }

    std::string userWinnerQuery = "SELECT * FROM ladder WHERE slackID = " + winnerID + ";";
    std::string userLooserQuery = "SELECT * FROM ladder WHERE slackID = " + looserID + ";";
    std::string winnerName;
    std::string looserName;
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, userWinnerQuery.c_str(), -1, &stmt, nullptr);

    if (rc == 0) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            winnerName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to execute query: " << sqlite3_errmsg(db) << std::endl;
    }
    rc = sqlite3_prepare_v2(db, userLooserQuery.c_str(), -1, &stmt, nullptr);

    if (rc == 0) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            looserName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to execute query: " << sqlite3_errmsg(db) << std::endl;
    }
    std::time_t t = std::time(nullptr);

    Match match(winnerName, 3, looserName, looserScore, *std::localtime(&t));
    recordMatch(match);
    winnerID = winnerID.substr(1, winnerID.size() - 2);
    looserID = looserID.substr(1, looserID.size() - 2);
    toSend = "<@" + winnerID + "> beat <@" + looserID + "> with a score of 3-" + std::to_string(looserScore);

    slack->chat.channel = slackChannelName;

    auto json = R"({
    "text":         "x",
    "channel":      "x"
})"_json;
    json["text"] = toSend;
    json["channel"] = slackChannelName;
    slack::post("chat.postMessage", json);
}


void Ladder::executeSQL(const char *sql) {
    char *errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);

    if (rc != 0) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        exit(1);
    }
}

void Ladder::createTables() {
    const char *createLadderTableSQL = "CREATE TABLE IF NOT EXISTS ladder ("
                                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                       "name TEXT NOT NULL,"
                                       "ranking INTEGER NOT NULL,"
                                       "slackID TEXT NOT NULL);";

    const char *createMatchHistoryTableSQL = "CREATE TABLE IF NOT EXISTS match_history ("
                                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                             "winner TEXT NOT NULL,"
                                             "loser TEXT NOT NULL,"
                                             "date TEXT NOT NULL,"
                                             "score TEXT NOT NULL);";

    executeSQL(createLadderTableSQL);
    executeSQL(createMatchHistoryTableSQL);
}

void Ladder::loadLadder() {
    const char *ladderQuery = "SELECT * FROM ladder ORDER BY ranking;";
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, ladderQuery, -1, &stmt, nullptr);

    if (rc == 0) {
        // Clear existing ladder data
        ladder.clear();

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            int ranking = sqlite3_column_int(stmt, 2);
            std::string slackID = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
            ladder.emplace_back(name, ranking, slackID);
        }

        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to execute query: " << sqlite3_errmsg(db) << std::endl;
        exit(1);
    }
}


void Ladder::saveMatchToDatabase(Match &match) {
    std::string insertMatchSQL =
            "INSERT INTO match_history (winner, loser, date, score) VALUES ('" + match.getWinnerName() + "', '" +
            match.getLooserName() + "', '" + match.getStringDate() + +"', '" + match.getStringScore() + "');";
    executeSQL(insertMatchSQL.c_str());
}

void Ladder::addPlayer(const std::string &name, const std::string &slackUser) {
    ladder.emplace_back(name, ladder.size() + 1, slackUser);
    std::string insertMatchSQL = "INSERT INTO ladder (name, ranking, slackID) VALUES ('" + name + "', '" +
                                 std::to_string(ladder.size() + 1) + "', '" + slackUser + "');";
    executeSQL(insertMatchSQL.c_str());
}

void Ladder::updateLadder() {
    for (const Player &player: ladder) {
        std::string updateSQL =
                "UPDATE ladder SET ranking = " + std::to_string(player.getRanking()) + " WHERE name = '" +
                player.getName() + "';";
        executeSQL(updateSQL.c_str());
    }
}

void Ladder::recordMatch(Match matchPlayed) {
    matchHistory.emplace_back(matchPlayed);
    saveMatchToDatabase(matchPlayed);

    auto winnerIt = std::find_if(ladder.begin(), ladder.end(),
                                 [&matchPlayed](const Player &player) {
                                     return player.getName() == matchPlayed.getWinnerName();
                                 });

    auto loserIt = std::find_if(ladder.begin(), ladder.end(),
                                [&matchPlayed](const Player &player) {
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
    std::sort(ladder.begin(), ladder.end(), [](const Player &a, const Player &b) {
        return a.getRanking() < b.getRanking();
    });
    updateLadder(); // not the most efficient to do it here, but makes sure the this.ladder matches the db
}

void Ladder::printMatches() {
    std::cout << "Matches recorded:" << std::endl;
    for (const Match &match: matchHistory) {
        std::cout << match;
    }
}

void Ladder::createSlackConnection() {
    const std::string endOfURI = performSocketCurlCheck(appToken);
    std::string host = "wss-primary.slack.com";
    const char *port = "443";
    const char *text = endOfURI.c_str();
    try {
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
        beast::flat_buffer buffer;
        ws.read(buffer); // this is hello message from slack
        //TODO if this is not a hello message, try connection again
        std::cout << beast::make_printable(buffer.data()) << std::endl;
        buffer.consume(buffer.size()); // clears buffer
    }
    catch (std::exception const &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        exit(1);
    }
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *output) {
    size_t total_size = size * nmemb;
    output->append((char *) contents, total_size);
    return total_size;
}

std::string Ladder::performSocketCurlCheck(const std::string &token) {
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
