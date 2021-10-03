//
// Created by baifeng on 2021/9/26.
//

#ifndef SDL2_UI_VARIABLE_H
#define SDL2_UI_VARIABLE_H

#include <memory>

class Fps;
class ScreenWidget;
class GamePad;
class AudioSystem;
class EventCenter;
class GameVariable {
public:
    GameVariable();
    virtual ~GameVariable();
public:
    Fps& fps();
    ScreenWidget& screen();
    GamePad& gamepad();
    AudioSystem& audio();
    EventCenter& event();
protected:
    void initVariable();
protected:
    std::shared_ptr<Fps> _fps;
    std::shared_ptr<ScreenWidget> _screen;
    std::shared_ptr<GamePad> _gamepad;
    std::shared_ptr<AudioSystem> _audio;
    std::shared_ptr<EventCenter> _event;
};

#endif //SDL2_UI_VARIABLE_H
