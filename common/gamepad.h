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
#include "types.h"

class GamePadListener {
public:
    typedef enum {
        JOY1 = 0,
        JOY2,
        L2,
        R2,
    } JOYIDX;
    typedef enum {
        UNKNOWN,
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
    typedef std::unordered_map<int, bool> KeyState;
public:
    GamePad();
public:
    bool isPressed(int key) const;
    void add(WidgetPtr const& widget);
    void remove(WidgetPtr const& widget);
    void remove(Widget const* widget);
    void onEvent(SDL_Event const& event);
private:
    void _onJoyEvent(SDL_Event const& event);
    void _remapKeyboardEvent(SDL_Event const& event);
private:
    bool _keyboard_event;
    WidgetPtr _widget;
    KeyMap _keyValue;
    KeyState _keyState;
    Vector2i _joyValue[2];
    List _views;
};

#endif //SDL2_UI_GAMEPAD_H
