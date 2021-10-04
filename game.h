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

class MyGame : public Game::App {
public:
    void init() override {
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
    }
};

#endif //SDL2_UI_GAME_H
