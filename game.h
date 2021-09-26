//
// Created by baifeng on 2021/7/12.
//

#ifndef SDL2_UI_GAME_H
#define SDL2_UI_GAME_H

#include "common/game.h"
#include "common/loadres.h"
#include "common/render.h"
#include "common/view.h"
#include "common/action.h"
#include <list>

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

        {
            auto zoom_big = Action::Ptr(new ScaleBy(bgView.get(), {1.0f, 1.0f}, 0.5f));
            auto zone_small = Action::Ptr(new ScaleBy(bgView.get(), {-1.0f, -1.0f}, 0.5f));
            auto action_seq = Action::Ptr(new Sequence({zoom_big, zone_small}));
            auto repeat = Action::Ptr(new Repeat(action_seq));
            bgView->runAction(repeat);
            bgView->setScale({0.5f, 0.5f});
            bgView->enableUpdate(true);
        }

        {
            auto move1 = Action::Ptr(new MoveBy(iconView.get(), {240, 0}, 1.0f));
            auto move2 = Action::Ptr(new MoveBy(iconView.get(), {-240, 0}, 1.0f));
            auto seq = Action::Ptr(new Sequence({move1, move2}));
            auto repeat = Action::Ptr(new Repeat(seq));
            iconView->runAction(repeat);
            iconView->enableUpdate(true);
            iconView->setPosition(480, 272);
        }
    }
    void update(float delta) override {
        root.update(delta);
    }
    void render(SDL_Renderer* renderer) override {
        root.draw(renderer);
    }
    void fini() override {
    }
};

#endif //SDL2_UI_GAME_H
