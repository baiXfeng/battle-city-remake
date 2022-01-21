//
// Created by baifeng on 2021/11/14.
//

#ifndef SDL2_UI_EVENTS_H
#define SDL2_UI_EVENTS_H

#include "box2d/box2d.h"
#include "entity.h"

namespace entity {
    class Context;
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
    struct EntityPhysicsContact {
        entity::Context* c;
        b2Contact* contact;
        b2Fixture* sender;
        b2Fixture* target;
    };
}

#endif //SDL2_UI_EVENTS_H
