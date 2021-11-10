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
#include "const.h"
#include "sound_effect.h"
#include "skin.h"

void GameOverCommand::onEvent(Event const& e) {

    _game.gamepad().sleep(60.0f);

    std::string const box_name = "gameover:box";
    auto& target = e.data<Widget*>();
    if (target->find(box_name)) {
        return;
    }

    auto box = Widget::New<WindowWidget>();
    box->setPosition(target->size().x * 0.5f, target->size().y * 1.1f);
    box->setAnchor(0.5f, 0.5f);
    box->setName(box_name);
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

void PlayerWinCommand::onEvent(Event const& e) {
    auto scene = _game.screen().scene_back();
    scene->defer([](){
        _game.gamepad().sleep(60.0f);
        _game.event().notify(Event(EventID::PLAYER_STOP_CONTROL));
    }, 3.0f);
    scene->defer([]{
        _game.screen().replace<ScoreView>();
    }, 4.0f);
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

    _SE.playSE(_SE.PAUSE_SE);
}

void ResumeGameCommand::onEvent(Event const& e) {

    auto& target = e.data<Widget*>();
    target->parent()->removeChild(target->parent()->find("pause_label"));
    target->resumeAllActions();
    target->enableUpdate(true);

    _SE.stopSE(_SE.PAUSE_SE);
}

void LoadResCommand::onEvent(Event const& e) {
    skin::getShieldSkin();
    skin::getTankAppearSkin();
    for (int i = Tank::A; i < Tank::TIER_MAX; ++i) {
        auto tier = Tank::Tier(i);
        skin::getDEnemySkin(tier);
        skin::getEnemySkin(tier, false);
        skin::getEnemySkin(tier, true);
        skin::getPlayerSkin(tier, Tank::P1);
    }
    _SE.load();
}
