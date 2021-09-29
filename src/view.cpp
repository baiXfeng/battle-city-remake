//
// Created by baifeng on 2021/9/29.
//

#include "view.h"
#include "common/loadres.h"
#include "common/game.h"
#include "common/action.h"

LogoView::LogoView():_canClick(false) {
    auto bg = res::load_texture(_game.renderer(), "assets/images/logo.png");
    auto image = New<ImageWidget>(bg);
    auto mask = New<MaskWidget>(SDL_Color{0, 0, 0, 255});
    addChild(image);
    addChild(mask);
    _mask = mask->to<MaskWidget>();
}

void LogoView::onButtonDown(int key) {
    if (_canClick) {
        this->stopAllActions();
        auto fadeout = Action::Ptr(new ProgressAction(std::bind(&LogoView::onFadeOut, this, std::placeholders::_1), 0.5f));
        auto delay1 = Action::New<Delay>(1.0f);
        auto callback = Action::Ptr(new CallBackVoid(_callback));
        auto action = Action::Ptr(new Sequence({fadeout, delay1, callback}));
        this->runAction(action);
        _canClick = false;
    }
}

void LogoView::setFinishCall(Callback const& cb) {
    _callback = cb;
}

void LogoView::onEnter() {
    GamePadWidget::onEnter();
    auto fadein = Action::Ptr(new ProgressAction(std::bind(&LogoView::onFadeIn, this, std::placeholders::_1), 0.5f));
    auto delay1 = Action::Ptr(new Delay(1.0f));
    auto clickon = Action::New<CallBackVoid>([&]{
        this->_canClick = true;
    });
    auto delay2 = Action::Ptr(new Delay(1.0f));
    auto clickoff = Action::New<CallBackVoid>([&]{
        this->_canClick = false;
    });
    auto fadeout = Action::Ptr(new ProgressAction(std::bind(&LogoView::onFadeOut, this, std::placeholders::_1), 0.5f));
    auto delay3 = Action::New<Delay>(1.0f);
    auto callback = Action::Ptr(new CallBackVoid(_callback));
    auto action = Action::Ptr(new Sequence({fadein, delay1, clickon, delay2, clickoff, fadeout, delay3, callback}));
    this->runAction(action);
}

void LogoView::onFadeIn(float v) {
    _mask->setOpacity(255 * (1-v));
}

void LogoView::onFadeOut(float v) {
    _mask->setOpacity(255 * v);
}