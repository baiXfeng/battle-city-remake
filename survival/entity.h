//
// Created by baifeng on 2021/11/14.
//

#ifndef SDL2_UI_ENTITY_H
#define SDL2_UI_ENTITY_H

#include "entt/entt.hpp"

namespace mge {
    class Widget;
}

namespace entity {
    typedef entt::entity id;
    typedef entt::registry world;
    typedef mge::Widget* view;
}

#endif //SDL2_UI_ENTITY_H
