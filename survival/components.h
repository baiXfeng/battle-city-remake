//
// Created by baifeng on 2021/11/14.
//

#ifndef SDL2_UI_COMPONENTS_H
#define SDL2_UI_COMPONENTS_H

#include "common/vector2.h"
#include "box2d/box2d.h"
#include "entity.h"

namespace component {

    struct player {};
    struct enemy {};
    struct tank {};
    struct bullet {};
    struct killed {};

    struct skin {
        entity::view view;
    };
    struct move_state {
        float rotation;
        mge::Vector2f speed;
    };
    struct fire_state {
        bool fire;
        float cooldown;
    };
    struct tank_brake {
        bool value;
    };
    struct lifetime {
        float value;
    };
    struct physics {
        b2Body* body;
    };
    enum class entity_type {
        TANK = 1,
        BULLET = 2,
    };
    struct entity_info {
        entity::id owner;
        entity::id sender;
        entity_type type;
    };

}

#endif //SDL2_UI_COMPONENTS_H
