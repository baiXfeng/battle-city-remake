//
// Created by baifeng on 2021/9/26.
//

#include "variable.h"
#include "fps.h"
#include "widget.h"
#include "gamepad.h"
#include "audio.h"
#include "mouse.h"
#include "command.h"

mge_begin

void GameVariable::initVariable() {
    _fps = std::make_shared<Fps>();
    _audio = std::make_shared<AudioSystem>();
    _event = std::make_shared<EventCenter>();
    _command = std::make_shared<CommandCenter>();
    _mouse = std::make_shared<Mouse>();
    _gamepad = std::make_shared<GamePad>();
    _screen = std::make_shared<ScreenWidget>();
}

void GameVariable::finiVariable() {
    _screen = nullptr;
    _gamepad = nullptr;
    _mouse = nullptr;
    _command = nullptr;
    _event = nullptr;
    _audio = nullptr;
    _fps = nullptr;
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

Mouse& GameVariable::mouse() {
    return *_mouse.get();
}

AudioSystem& GameVariable::audio() {
    return *_audio.get();
}

EventCenter& GameVariable::event() {
    return *_event.get();
}

CommandCenter& GameVariable::command() {
    return *_command.get();
}

mge_end
