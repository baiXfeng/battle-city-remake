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
#include <list>
#include "src/view.h"
#include "src/test_quadtree.h"
#include "common/proxy.h"
#include "lutok3.h"
#include "src/luafunc.h"

class Label : public TTFLabel {
public:
    Label(std::string const& text) {
        setFont(res::load_ttf_font("assets/fonts/prstart.ttf", 18));
        font()->setColor({255, 255, 255, 255});
        setAnchor(0.5f, 0.5f);
        setString(text);
    }
};

class MyGame : public Game::App {
public:
    void initData() {
        auto& state = _game.force_get<lutok3::State>("lua");
        state.openLibs();

        state.doFile(res::levelName("level_info"));

        // 记录最大关卡数
        state.getGlobal("LEVEL_MAX");
        int value = state.get( );
        state.pop();
        _game.force_get<int>("level_max") = value == 0 ? 1 : value;
        _game.force_get<int>("level") = 1;

        registerLuaFunctions(state);
    }
    void init() override {
        this->initData();
        auto view = firstScene();
        _game.screen().push(view);
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
    }
};

#endif //SDL2_UI_GAME_H
