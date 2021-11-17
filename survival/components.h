//
// Created by baifeng on 2021/11/14.
//

#ifndef SDL2_UI_COMPONENTS_H
#define SDL2_UI_COMPONENTS_H

#include "common/types.h"
#include "entity.h"
#include <memory>

namespace component {

    struct player {};
    struct enemy {};
    struct transform {
        float rotation;
        mge::Vector2f position;
    };
    struct moveable {
        mge::Vector2f speed;
    };
    struct weapon {
        float rotation;
    };
    struct skin {
        entity::View container;
        entity::View body;
        entity::View weapon;
    };

}

#endif //SDL2_UI_COMPONENTS_H
