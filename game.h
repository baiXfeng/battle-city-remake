//
// Created by baifeng on 2021/7/12.
//

#ifndef SDL2_UI_GAME_H
#define SDL2_UI_GAME_H

#include "common/game.h"
#include "common/loadres.h"
#include "common/render.h"
#include "common/view.h"

class MyGame : public Game::App {
private:
    WindowWidget root;

public:
    void init() override {

        auto renderer = _game.renderer();
        auto bg = res::load_texture(renderer, "assets/bg.jpg");

        auto bgView = Widget::Ptr(new ImageWidget(bg));
        root.addChild(bgView);

        auto iconView = Widget::Ptr(new ImageWidget(bg));
        iconView->setScale(0.5f, 0.5f);
        iconView->setPosition(480, 272);
        root.addChild(iconView);

        auto maskView = Widget::Ptr(new MaskWidget({255, 0, 0, 40}));
        root.addChild(maskView);
    }
    void update(float delta) override {
        root.update(delta);
    }
    void render(SDL_Renderer* renderer) override {
        //bg_render.draw(renderer);
        root.draw(renderer);
    }
    void fini() override {
    }
};

#endif //SDL2_UI_GAME_H
