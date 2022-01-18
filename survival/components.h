//
// Created by baifeng on 2021/11/14.
//

#ifndef SDL2_UI_COMPONENTS_H
#define SDL2_UI_COMPONENTS_H

#include "common/vector2.h"
#include "entity.h"

namespace component {

    struct player {};
    struct enemy {};
    struct bullet {};
    struct killed {};

    struct skin {
        entity::view view;
    };
    struct move_speed {
        mge::Vector2f value;
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

}

#endif //SDL2_UI_COMPONENTS_H
