//
// Created by baifeng on 2021/11/14.
//

#ifndef SDL2_UI_CONTEXT_H
#define SDL2_UI_CONTEXT_H

#include "entity.h"

namespace entity {
    class Context {
    public:
        float delta;
        entt::dispatcher dispatcher;
        entity::world world;
        entity::View rootView;
    };
}

#endif //SDL2_UI_CONTEXT_H
