//
// Created by baifeng on 2021/10/4.
//

#include "mouse.h"
#include "game.h"

mge_begin

Mouse::Mouse() {

}

void Mouse::sleep(float seconds) {

}

void Mouse::add(WidgetPtr const& widget) {

}

void Mouse::remove(WidgetPtr const& widget) {

}

void Mouse::remove(Widget const* widget) {

}

void Mouse::onEvent(SDL_Event const& event) {
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
            break;
        case SDL_MOUSEBUTTONUP:
            break;
        case SDL_MOUSEMOTION:
            break;
        default:
            break;
    }
}

void Mouse::onFingerEvent(Event e, Vector2i const& postion) {
    switch (e) {
        case FINGER_DOWN:
            onFingerDown(postion);
            break;
        case FINGER_UP:
            onFingerUp(postion);
            break;
        case FINGER_MOTION:
            onFingerMotion(postion);
            break;
        default:
            break;
    }
}

void Mouse::onFingerDown(Vector2i const& postion) {

}

void Mouse::onFingerUp(Vector2i const& postion) {

}

void Mouse::onFingerMotion(Vector2i const& postion) {

}

mge_end
