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
#include "survival/world.h"
#include "survival/menu.h"
#include "dungeon/mini_span_tree.h"
#include "lutok3.h"
#include "uilayout/ui-layout.h"
#include "common/xml_layout.h"
#include <algorithm>

class MyWidget : public mge::WindowWidget, public ui::LayoutVariableAssigner, public ui::LayoutNodeListener {
public:
    MyWidget() {

    }
private:
    bool onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) override {
        UI_LAYOUT_MEMBER_ASSIGN(this, "imgIcon", mge::ImageWidget*, _icon);
        UI_LAYOUT_MEMBER_ASSIGN(this, "test2", mge::Widget*, _second);
        return false;
    }
    void onLayoutLoaded() override {

    }
private:
    mge::ImageWidget* _icon;
    mge::Widget* _second;
};

class MyWidgetLoader : public ui::NodeLoader {
    UI_NODE_LOADER_CREATE(MyWidget);
};

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
        //_game.screen().push<RandomRoomView>();
        _game.screen().push<SurvivalView>();

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
    mge::Vector2i screenSize() override {
        //return {1280, 960};
        return App::screenSize();
    }
};

#endif //SDL2_UI_GAME_H
