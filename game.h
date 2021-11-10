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
#include "time.h"

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
        int level_max = Tank::getGlobalInt("LEVEL_MAX");
        _game.force_get<int>("level_max") = level_max == 0 ? 1 : level_max;
        _game.force_get<int>("level") = 1;
    }
    void initData() {
        initLuaState();
        initLevelMax();
        Tank::loadTankSpawns();
        Tank::loadAttributes();
    }
    void initCommand() {
        _game.command().add<GameOverCommand>(EventID::GAME_OVER_ANIMATION);
        _game.command().add<PauseGameCommand>(EventID::PAUSE_GAME);
        _game.command().add<ResumeGameCommand>(EventID::RESUME_GAME);
        _game.command().add<PlayerWinCommand>(EventID::PLAYER_WIN);
        _game.command().add<LoadResCommand>(EventID::LOAD_RES);
    }
    void init() override {
        LOG_INIT();
        srand(time(nullptr));
        this->initData();
        this->initCommand();
#ifdef WIN32
        _game.screen().push<StartView>();
#else
        _game.screen().push<LogoView>();
#endif

#if defined(__PSP__)
        SDL_RenderSetScale(_game.renderer(), 0.5f, 0.5f);
#endif
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
