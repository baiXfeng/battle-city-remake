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
#include <list>

class HelloWorld : public GamePadWidget {
public:
    ~HelloWorld() {
        
    }
    HelloWorld() {
        auto renderer = _game.renderer();
        auto bg = res::load_texture(renderer, "assets/bg.jpg");
        auto root = this;

        auto bgView = Widget::Ptr(new ImageWidget(bg));
        root->addChild(bgView);

        auto iconView = Widget::Ptr(new ImageWidget(bg));
        iconView->setScale(0.5f, 0.5f);
        iconView->setPosition(480, 272);
        root->addChild(iconView);

        auto maskView = Widget::Ptr(new MaskWidget({255, 0, 0, 40}));
        root->addChild(maskView);

        {
            auto call = Action::New<CallBackVoid>([&]{
                auto a1 = Action::Ptr(new PushSceneAction<HelloWorld>(true));
                auto a1_delay = Action::Ptr(new Delay(1.0f));
                auto a1_seq = Action::Ptr(new Sequence({a1, a1_delay}));
                auto a2 = Action::Ptr(new CallBackVoid([]() {
                    //printf("开幕.\n");
                }));
                _game.screen().cut_to(a1_seq, 0.33f);
            });
            auto delay = Action::Ptr(new Delay(2.0f));
            auto seq = Action::Ptr(new Sequence({delay, call}));
            auto action = Action::Ptr(new Repeat(seq));
            root->runAction(action);
        }

        {
            auto zoom_big = Action::Ptr(new ScaleBy(bgView.get(), {1.0f, 1.0f}, 0.5f));
            auto zone_small = Action::Ptr(new ScaleBy(bgView.get(), {-1.0f, -1.0f}, 0.5f));
            auto action_seq = Action::Ptr(new Sequence({zoom_big, zone_small}));
            auto repeat = Action::Ptr(new Repeat(action_seq));
            bgView->runAction(repeat);
            bgView->setScale({0.5f, 0.5f});
        }

        {
            auto move1 = Action::Ptr(new MoveBy(iconView.get(), {240, 0}, 1.0f));
            auto move2 = Action::Ptr(new MoveBy(iconView.get(), {-240, 0}, 1.0f));
            auto seq = Action::Ptr(new Sequence({move1, move2}));
            auto repeat = Action::Ptr(new Repeat(seq));
            iconView->runAction(repeat);
            iconView->setPosition(480, 272);
            iconView->setAnchor({0.5f, 0.5f});
        }

        {
            auto blink = Action::Ptr(new Blink(iconView.get(), 1.0f));
            auto repeat = Action::Ptr(new Repeat(blink));
            iconView->runAction(repeat);
        }

        gamepad_sleep(1.0f);
    }
private:
    void onButtonDown(int key) override {
        if (key == KeyCode::X) {
            _game.screen().replace<HelloWorld>();
            //_game.screen().pop();
        }
    }
};

class MyGame : public Game::App {
public:
    void init() override {
        _game.screen().push<HelloWorld>();
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
