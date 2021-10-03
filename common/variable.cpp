//
// Created by baifeng on 2021/9/26.
//

#include "variable.h"
#include "fps.h"
#include "widget.h"
#include "gamepad.h"
#include "audio.h"

GameVariable::GameVariable() {

}

GameVariable::~GameVariable() {
    _screen = nullptr;
    _gamepad = nullptr;
    _event = nullptr;
    _audio = nullptr;
    _fps = nullptr;
}

void GameVariable::initVariable() {
    _fps = std::make_shared<Fps>();
    _audio = std::make_shared<AudioSystem>();
    _event = std::make_shared<EventCenter>();
    _gamepad = std::make_shared<GamePad>();
    _screen = std::make_shared<ScreenWidget>();
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

EventCenter& GameVariable::event() {
    return *_event.get();
}
