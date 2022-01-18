//
// Created by baifeng on 2021/9/27.
//

#ifndef SDL2_UI_GAMEPAD_H
#define SDL2_UI_GAMEPAD_H

#include <list>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <SDL.h>
#include "macro.h"
#include "vector2.h"

mge_begin

class GamePadListener {
public:
    typedef enum {
        JOY1 = 0,
        JOY2,
        L2,
        R2,
    } JOYIDX;
    typedef enum {
        UNKNOWN = 0,
        UP,
        DOWN,
        LEFT,
        RIGHT,
        L1,
        R1,
        SELECT,
        START,
        A,
        B,
        X,
        Y,
        MAX,
    } KeyCode;
public:
    virtual ~GamePadListener() {}
public:
    virtual void onButtonDown(int key) {}
    virtual void onButtonUp(int key) {}
    virtual void onJoyAxisMotion(JOYIDX joy_id, int x, int y) {}
};

class Widget;
class GamePad {
public:
    typedef std::shared_ptr<Widget> WidgetPtr;
    typedef std::list<WidgetPtr> List;
    typedef std::vector<int> KeyMap;
    typedef std::vector<bool> KeyState;
    typedef GamePadListener::KeyCode KeyCode;
public:
    GamePad();
public:
    void sleep(float seconds);
    bool isPressed(int key) const;
    void add(WidgetPtr const& widget);
    void remove(WidgetPtr const& widget);
    void remove(Widget const* widget);
    void onEvent(SDL_Event const& event);
private:
    void _onJoyEvent(SDL_Event const& event);
    void _remapKeyboardEvent(SDL_Event const& event);
private:
    bool _sleep;
    bool _keyboard_event;
    KeyMap _keyValue;
    KeyState _keyState;
    Vector2i _joyValue[2];
    List _views;
};

mge_end

#endif //SDL2_UI_GAMEPAD_H
