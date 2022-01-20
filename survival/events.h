//
// Created by baifeng on 2021/11/14.
//

#ifndef SDL2_UI_EVENTS_H
#define SDL2_UI_EVENTS_H

namespace component {
    struct entity_info;
}

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
    struct EntityBeginTouch {
        component::entity_info* infoA;
        component::entity_info* infoB;
    };
    struct EntityEndTouch {
        component::entity_info* infoA;
        component::entity_info* infoB;
    };
}

#endif //SDL2_UI_EVENTS_H
