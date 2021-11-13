//
// Created by baifeng on 2021/7/12.
//

#ifndef SDL2_UI_GAME_H
#define SDL2_UI_GAME_H

#include "common/game.h"
#include "common/log.h"
#include "common/command.h"
#include "common/widget.h"
#include "src/command.h"
#include "src/const.h"
#include "src/view.h"
#include "lutok3.h"

class MyGame : public mge::Game::App {
private:
    lutok3::State* _state;
public:
    MyGame():_state(&_game.force_get<lutok3::State>("lua")) {}
    void initCommand() {
        _game.command().add<BattleModeInitCommand>(EventID::BATTLE_CITY_INIT);
        _game.command().add<GameOverCommand>(EventID::GAME_OVER_ANIMATION);
        _game.command().add<PauseGameCommand>(EventID::PAUSE_GAME);
        _game.command().add<ResumeGameCommand>(EventID::RESUME_GAME);
        _game.command().add<PlayerWinCommand>(EventID::PLAYER_WIN);
        _game.command().add<LoadResCommand>(EventID::LOAD_RES);
    }
    void init() override {
        LOG_INIT();
        this->initCommand();
        _game.event().notify(mge::Event(EventID::BATTLE_CITY_INIT));
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
