//
// Created by baifeng on 2021/10/7.
//

#include "command.h"
#include "common/game.h"
#include "common/widget.h"
#include "common/loadres.h"
#include "common/action.h"
#include "common/audio.h"
#include "view.h"
#include "data.h"

void GameOverCommand::onEvent(Event const& e) {

    _game.gamepad().sleep(60.0f);

    auto& target = e.data<Widget*>();
    auto box = Widget::New<WindowWidget>();
    box->setPosition(target->size().x * 0.5f, target->size().y * 1.1f);
    box->setAnchor(0.5f, 0.5f);
    target->addChild(box);

    auto font = res::load_ttf_font(res::fontName("prstart"), 20);
    font->setColor({181, 49, 32, 255});

    std::string title[2] = {"GAME", "OVER"};
    for (int i = 0; i < 2; ++i) {
        auto label = TTFLabel::New(title[i], font);
        label->setPosition(0.0f, i * label->size().y + i * 4);
        box->addChild(label);
        box->setSize(label->size().x, label->size().y * 2 + 4);
    }

    auto move = Action::Ptr(new MoveTo(box.get(), {target->size().x * 0.5f, target->size().y * 0.5f}, 2.5f));
    auto delay = Action::New<Delay>(2.5f);
    auto call = Action::New<CallBackVoid>([]{
        _game.screen().replace<ScoreView>();
    });
    auto action = Action::Ptr(new Sequence({move, delay, call}));
    box->runAction(action);
    target->performLayout();
}

void PauseGameCommand::onEvent(Event const& e) {

    auto& target = e.data<Widget*>();
    auto font = res::load_ttf_font(res::fontName("prstart"), 18);
    font->setColor({189, 64, 48, 255});

    auto label = TTFLabel::New("PAUSE", font, {0.5f, 0.5f});
    label->setName("pause_label");
    label->setPosition(target->size().x * 0.5f, target->size().y * 0.5f);
    target->parent()->addChild(label);

    auto blink = Action::New<Blink>(label.get(), 1, 0.55f);
    auto repeat = Action::New<Repeat>(blink);
    label->runAction(repeat);

    target->pauseAllActions();
    target->enableUpdate(false);

    auto sound = res::soundName("pause");
    _game.audio().loadEffect(sound);
    _game.audio().playEffect(sound);
}

void ResumeGameCommand::onEvent(Event const& e) {

    auto& target = e.data<Widget*>();
    target->parent()->removeChild(target->parent()->find("pause_label"));
    target->resumeAllActions();
    target->enableUpdate(true);

    auto sound = res::soundName("pause");
    _game.audio().releaseEffect(sound);
}

void BulletHitTankCommand::onEvent(Event const& e) {

    auto& info = e.data<BulletHitTankInfo>();
    auto bullet = info.bullet;
    auto tank = info.tank;
    auto world = info.world;

    --tank->hp;
    if (tank->hp <= 0) {
        auto iter = std::find(world->tanks.begin(), world->tanks.end(), tank);
        world->tanks.erase(iter);
    }
    tank->createScore();
    tank->createExplosion();
    tank->removeFromScreen();
}
