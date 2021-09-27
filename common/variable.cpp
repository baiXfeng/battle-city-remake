//
// Created by baifeng on 2021/9/26.
//

#include "variable.h"
#include "fps.h"
#include "view.h"
#include "gamepad.h"
#include "audio.h"

GameVariable::GameVariable() {

}

void GameVariable::initVariable() {
    _fps = std::make_shared<Fps>();
    _screen = std::make_shared<ScreenWidget>();
    _gamepad = std::make_shared<GamePad>();
    _audio = std::make_shared<AudioSystem>();
}

Fps& GameVariable::fps() {
    return *_fps.get();
}

ScreenWidget& GameVariable::screen() {
    return *_screen.get();
}

GamePad& GameVariable::gamepad() {
    return *_gamepad.get();
}

AudioSystem& GameVariable::audio() {
    return *_audio.get();
}
