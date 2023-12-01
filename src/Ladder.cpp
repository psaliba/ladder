//
// Created by Peter Saliba on 12/1/23.
//

#include "../include/Ladder.h"

Ladder::Ladder() = default;

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

