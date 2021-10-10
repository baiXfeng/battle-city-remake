//
// Created by baifeng on 2021/9/27.
//

#include "gamepad.h"
#include "widget.h"
#include "game.h"
#include "action.h"
#include "log.h"

typedef GamePadListener::KeyCode KeyCode;
typedef GamePadListener::JOYIDX JOYIDX;

void initKeyMapVita(GamePad::KeyMap& km) {
    km.resize(12, KeyCode::UNKNOWN);
    KeyCode list[] = {
            KeyCode::X,
            KeyCode::A,
            KeyCode::B,
            KeyCode::Y,
            KeyCode::L1,
            KeyCode::R1,
            KeyCode::DOWN,
            KeyCode::LEFT,
            KeyCode::UP,
            KeyCode::RIGHT,
            KeyCode::SELECT,
            KeyCode::START
    };
    for (int i = 0; i < km.size(); ++i) {
        km[i] = list[i];
    }
}

GamePad::GamePad():_keyboard_event(false), _sleep(false) {
#if defined(__vita__)
    initKeyMapVita(_keyValue);
#endif
    _keyState.resize(KeyCode::MAX, false);
}

void GamePad::sleep(float seconds) {
    _sleep = true;
    auto delay = Action::Ptr(new Delay(seconds));
    auto call = Action::Ptr(new CallBackVoid([&]{
        this->_sleep = false;
    }));
    auto action = Action::Ptr(new Sequence({delay, call}));
    action->setName("GamePad::sleep");
    auto& widget = _game.screen();
    widget.stopAction(action->name());
    widget.runAction(action);
}

bool GamePad::isPressed(int key) const {
    return _keyState[key];
}

void GamePad::add(WidgetPtr const& widget) {
    this->remove(widget);
    _views.push_back(widget);
}

void GamePad::remove(WidgetPtr const& widget) {
    this->remove(widget.get());
}

void GamePad::remove(Widget const* widget) {
    for (auto iter = _views.begin(); iter != _views.end(); iter++) {
        if (iter->get() == widget) {
            if (_views.back() == *iter) {
                _keyState.clear();
            }
            _views.erase(iter);
            return;
        }
    }
}

#define GAMEPADKEY(key) (_keyboard_event ? key : (key < _keyValue.size() ? _keyValue[key] : KeyCode::UNKNOWN))

void GamePad::onEvent(SDL_Event const& event) {
    switch (event.type) {
        case SDL_JOYBUTTONUP:
        {
            int key = GAMEPADKEY(event.jbutton.button);
            if (not _keyState[key]) {
                return;
            }
            _keyState[key] = false;
            if (_views.size() and not _sleep) {
                _views.back()->onButtonUp(key);
            }
        }
            break;
        case SDL_JOYBUTTONDOWN:
        {
            int key = GAMEPADKEY(event.jbutton.button);
            if (_keyState[key]) {
                return;
            }
            _keyState[key] = true;
            if (_views.size() and not _sleep) {
                _views.back()->onButtonDown(key);
            }
        }
            break;
        case SDL_JOYAXISMOTION:
            // 摇杆事件
            if (not _sleep) {
                this->_onJoyEvent(event);
            }
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            // 键盘事件
            this->_remapKeyboardEvent(event);
            break;
        default:
            break;
    }
}

//模拟手柄死区
const int JOYSTICK_DEAD_ZONE = 8000;

void GamePad::_onJoyEvent(SDL_Event const& e) {
    //Motion on controller 0
    if( e.jaxis.which == 0 ) {
        auto value = (e.jaxis.value / 32768.0f) * 128;
        JOYIDX idx;
        switch (e.jaxis.axis) {
            case 0:
                idx = JOYIDX::JOY1;
                _joyValue[0].x = value;
                break;
            case 1:
                idx = JOYIDX::JOY1;
                _joyValue[0].y = value;
                break;
            case 2:
                idx = JOYIDX::JOY2;
                _joyValue[1].x = value;
                break;
            case 3:
                idx = JOYIDX::JOY2;
                _joyValue[1].y = value;
                break;
            default:
                return;
        }
        if (_views.size()) {
            //LOG("joy: %d, x: %d, y: %d\n", idx, _joyValue[idx].x, _joyValue[idx].y);
            _views.back()->onJoyAxisMotion(idx, _joyValue[idx].x, _joyValue[idx].y);
        }
    }
}

void GamePad::_remapKeyboardEvent(SDL_Event const& e) {
    static std::map<int, int> keymap = {
            {SDL_SCANCODE_W, KeyCode::UP},
            {SDL_SCANCODE_S, KeyCode::DOWN},
            {SDL_SCANCODE_A, KeyCode::LEFT},
            {SDL_SCANCODE_D, KeyCode::RIGHT},
            {SDL_SCANCODE_L, KeyCode::A},
            {SDL_SCANCODE_K, KeyCode::B},
            {SDL_SCANCODE_I, KeyCode::X},
            {SDL_SCANCODE_J, KeyCode::Y},
            {SDL_SCANCODE_C, KeyCode::SELECT},
            {SDL_SCANCODE_N, KeyCode::START},
            {SDL_SCANCODE_U, KeyCode::L1},
            {SDL_SCANCODE_O, KeyCode::R1},
    };
    SDL_Event event;
    if (e.key.state == 1) {
        // 按下
        event.jbutton.button = keymap[e.key.keysym.scancode];
        event.type = SDL_JOYBUTTONDOWN;
    } else if (e.key.state == 0) {
        // 弹起
        event.jbutton.button = keymap[e.key.keysym.scancode];
        event.type = SDL_JOYBUTTONUP;
    }
    _keyboard_event = true;
    this->onEvent(event);
    _keyboard_event = false;
}