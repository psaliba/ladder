//#define BOOST_ASIO_ENABLE_HANDLER_TRACKING

#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <memory>
#include <curl/curl.h>
#include "include/socket.hpp"
#include <iostream>
#include <fstream>
#include "include/json.hpp"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using json = nlohmann::json;

//------------------------------------------------------------------------------
size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *output) {
    size_t total_size = size * nmemb;
    output->append((char *) contents, total_size);
    return total_size;
}

int main(int argc, char **argv) {
    std::string xAppToken;
    std::ifstream infile("secret.txt");
    std::getline(infile, xAppToken);
    infile.close();

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
    std::string tokenHeader = "Authorization: Bearer " + xAppToken;
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
    const char *host = "wss-primary.slack.com";
    const char *port = "443";
    const char *text = endOfURI.c_str();

    // The io_context is required for all I/O
    net::io_context ioc;

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12_client};

    // Launch the asynchronous operation
    std::make_shared<session>(ioc, ctx)->run(host, port, text);

    // Run the I/O service. The call will return when
    // the socket is closed.
    ioc.run();

    return EXIT_SUCCESS;
}