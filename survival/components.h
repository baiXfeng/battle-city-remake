//
// Created by baifeng on 2021/11/14.
//

#ifndef SDL2_UI_COMPONENTS_H
#define SDL2_UI_COMPONENTS_H

#include "common/vector2.h"
#include "box2d/box2d.h"
#include "entity.h"

namespace event {
    struct EntityPhysicsContact;
}

namespace component {

    struct player {};
    struct enemy {};
    struct tank {};
    struct bullet {};
    struct wall {};
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
    struct owner_info {
        entity::id entity;
    };

    struct physics_layer {};
    struct physics_layer_visible {
        bool value;
    };
    typedef void (*CollisionHandler)(event::EntityPhysicsContact const& e);
    struct physics_collision_handler {
        CollisionHandler begin;
        CollisionHandler end;
    };

}

#endif //SDL2_UI_COMPONENTS_H
