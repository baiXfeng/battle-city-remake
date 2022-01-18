//
// Created by baifeng on 2021/11/14.
//

#ifndef SDL2_UI_EVENTS_H
#define SDL2_UI_EVENTS_H

namespace event {
    struct GamepadDown {
        int key;
    };
    struct GamepadUp {
        int key;
    };
    struct GamepadAxisMotion {
        int joy_id;
        int x, y;
    };
}

#endif //SDL2_UI_EVENTS_H
