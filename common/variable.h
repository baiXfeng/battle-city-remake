//
// Created by baifeng on 2021/9/26.
//

#ifndef SDL2_UI_VARIABLE_H
#define SDL2_UI_VARIABLE_H

#include <memory>
#include <string>
#include "proxy.h"

mge_begin

class Fps;
class ScreenWidget;
class GamePad;
class Mouse;
class AudioSystem;
class EventCenter;
class CommandCenter;
class GameVariable : public Proxy<std::string> {
public:
    Fps& fps();
    ScreenWidget& screen();
    GamePad& gamepad();
    Mouse& mouse();
    AudioSystem& audio();
    EventCenter& event();
    CommandCenter& command();
protected:
    void initVariable();
    void finiVariable();
protected:
    std::shared_ptr<Fps> _fps;
    std::shared_ptr<ScreenWidget> _screen;
    std::shared_ptr<GamePad> _gamepad;
    std::shared_ptr<Mouse> _mouse;
    std::shared_ptr<AudioSystem> _audio;
    std::shared_ptr<EventCenter> _event;
    std::shared_ptr<CommandCenter> _command;
};

mge_end

#endif //SDL2_UI_VARIABLE_H
