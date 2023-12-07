## Ladder
Ladder is a cpp app that tracks a teams ladder order. It integrates
with slack, so that team members can see the current ladder and 
enter new matches- all while staying in slack.

It uses sqlite to track the ladder and store match history, and 
relies heavily on boost beast for the socket connection to slack. 

### Libraries used
- [slacking](https://github.com/coin-au-carre/slacking). Used to send messages to a slack channel
    outside of a socket response.
- [nlohmann/json](https://github.com/nlohmann/json). Used to parse and create requests to send to
  slack. 
- [boost/beast](https://github.com/boostorg/beast). Used for websocket networking.
- [sqlite3](https://www.sqlite.org/cintro.html). Used for persistent storage.

### Other dependencies
- OpenSSl- used for wss connection
- curl- used to query slack for socket address


### How to use
The app currently supports two slack slash commands, `/ladder` and `/match`
- `/ladder`
  - returns the current ladder to the slack channel it was called from.
- `/match`
  - when called, it shows an interactive message to the user. 
  The user inputs the match using the message, and when they click submit a message 
    is sent to the channel the command was called from. The message contains the result of the match, 
  

The app expects a file called `secret.txt` that contains the following (in order, each token on a new line)
- slack xapp-1 token for socket connection
- slack app id
-  slack client id
-  slack bot user oAuth token
-  the name of the channel to post to