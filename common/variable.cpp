//
// Created by baifeng on 2021/9/26.
//

#include "variable.h"
#include "fps.h"
#include "view.h"

GameVariable::GameVariable() {

}

void GameVariable::initVariable() {
    _fps = std::make_shared<Fps>();
    _screen = std::make_shared<ScreenWidget>();
}

Fps& GameVariable::fps() {
    return *_fps.get();
}

ScreenWidget& GameVariable::screen() {
    return *_screen.get();
}