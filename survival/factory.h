//
// Created by baifeng on 2021/11/14.
//

#ifndef SDL2_UI_FACTORY_H
#define SDL2_UI_FACTORY_H

#include "entity.h"

namespace factory {
    entity::id create_tank(entity::world& w, bool player);
    entity::id create_bullet(entity::world& w);
    entity::id create_physics_layer(entity::world& w);
}

#endif //SDL2_UI_FACTORY_H
