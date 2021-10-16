//
// Created by baifeng on 2021/7/12.
//

#ifndef SDL2_UI_GAME_H
#define SDL2_UI_GAME_H

#include "common/game.h"
#include "common/loadres.h"
#include "common/render.h"
#include "common/widget.h"
#include "common/action.h"
#include "common/quadtree.h"
#include "common/collision.h"
#include "common/event.h"
#include "common/command.h"
#include "common/proxy.h"
#include "common/log.h"
#include "src/view.h"
#include "src/test_quadtree.h"
#include "src/luafunc.h"
#include "src/command.h"
#include "src/const.h"
#include "lutok3.h"

class MyGame : public Game::App {
private:
    lutok3::State* _state;
public:
    MyGame():_state(&_game.force_get<lutok3::State>("lua")) {}
    void initLuaState() {
        // 初始化LUA虚拟机
        auto& state = *_state;
        state.openLibs();
        registerLuaFunctions(state);
        state.doFile(res::scriptName("startup"));
    }
    void initLevelMax() {
        // 记录最大关卡数
        auto& state = *_state;
        state.getGlobal("LEVEL_MAX");
        int value = state.get();
        state.pop();
        _game.force_get<int>("level_max") = value == 0 ? 1 : value;
        _game.force_get<int>("level") = 1;
    }
    void initData() {
        initLuaState();
        initLevelMax();
        Tank::loadTankSpawns();
        Tank::loadAttributes();
    }
    void initCommand() {
        _game.command().add<GameOverCommand>(EventID::GAME_OVER);
        _game.command().add<PauseGameCommand>(EventID::PAUSE_GAME);
        _game.command().add<ResumeGameCommand>(EventID::RESUME_GAME);
        _game.command().add<BulletHitTankCommand>(EventID::BULLET_HIT_TANK);
    }
    void init() override {
        LOG_INIT();
        srand(time(nullptr));
        this->initData();
        this->initCommand();
        _game.screen().push<BattleView>();
    }
    void update(float delta) override {
        _game.screen().update(delta);
    }
    void render(SDL_Renderer* renderer) override {
        _game.screen().render(renderer);
    }
    void fini() override {
        _game.screen().popAll();
        _game.remove("lua");
        LOG_FINI();
    }
};

#endif //SDL2_UI_GAME_H
