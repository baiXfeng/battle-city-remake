//
// Created by baifeng on 2021/10/7.
//

#ifndef BATTLE_CITY_SRC_COMMAND_H
#define BATTLE_CITY_SRC_COMMAND_H

#include "common/command.h"

class BattleModeInitCommand : public mge::Command {
    void onEvent(mge::Event const& e) override;
};

class GameOverCommand : public mge::Command {
    void onEvent(mge::Event const& e) override;
};

class PlayerWinCommand : public mge::Command {
    void onEvent(mge::Event const& e) override;
};

class PauseGameCommand : public mge::Command {
    void onEvent(mge::Event const& e) override;
};

class ResumeGameCommand : public mge::Command {
    void onEvent(mge::Event const& e) override;
};

class LoadResCommand : public mge::Command {
    void onEvent(mge::Event const& e) override;
};

#endif //BATTLE_CITY_SRC_COMMAND_H
