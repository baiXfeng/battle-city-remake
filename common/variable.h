//
// Created by baifeng on 2021/9/26.
//

#ifndef SDL2_UI_VARIABLE_H
#define SDL2_UI_VARIABLE_H

#include <memory>

class Fps;
class ScreenWidget;
class GameVariable {
public:
    GameVariable();
    virtual ~GameVariable() {}
public:
    Fps& fps();
    ScreenWidget& screen();
protected:
    void initVariable();
protected:
    std::shared_ptr<Fps> _fps;
    std::shared_ptr<ScreenWidget> _screen;
};

#endif //SDL2_UI_VARIABLE_H
