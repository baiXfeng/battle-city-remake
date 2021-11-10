//
// Created by baifeng on 2021/10/4.
//

#ifndef SDL2_UI_MOUSE_H
#define SDL2_UI_MOUSE_H

#include <list>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <SDL.h>
#include "types.h"

mge_begin

class Widget;
class Mouse {
public:
    typedef std::shared_ptr<Widget> WidgetPtr;
    typedef std::list<WidgetPtr> List;
    enum Event {
        FINGER_DOWN = 0,
        FINGER_UP,
        FINGER_MOTION,
    };
public:
    Mouse();
public:
    void sleep(float seconds);
    void add(WidgetPtr const& widget);
    void remove(WidgetPtr const& widget);
    void remove(Widget const* widget);
    void onEvent(SDL_Event const& event);
    void onFingerEvent(Event e, Vector2i const& postion);
private:
    void onFingerDown(Vector2i const& postion);
    void onFingerUp(Vector2i const& postion);
    void onFingerMotion(Vector2i const& postion);
private:
    bool _sleep;
    bool _finger_downed;
};

mge_end

#endif //SDL2_UI_MOUSE_H
