//
// Created by baifeng on 2021/10/7.
//

#ifndef BATTLE_CITY_SRC_COMMAND_H
#define BATTLE_CITY_SRC_COMMAND_H

#include "common/command.h"

class GameOverCommand : public Command {
    void onEvent(Event const& e) override;
};

class PlayerWinCommand : public Command {
    void onEvent(Event const& e) override;
};

class PauseGameCommand : public Command {
    void onEvent(Event const& e) override;
};

class ResumeGameCommand : public Command {
    void onEvent(Event const& e) override;
};

#endif //BATTLE_CITY_SRC_COMMAND_H
