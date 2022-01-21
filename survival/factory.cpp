//
// Created by baifeng on 2021/11/14.
//

#include "factory.h"
#include "components.h"

namespace factory {

    entity::id create_tank(entity::world& w, bool player) {
        auto const e = w.create();
        w.emplace<component::tank>(e);
        w.emplace<component::tank_brake>(e, false);
        w.emplace<component::skin>(e, nullptr);
        w.emplace<component::move_state>(e, 0.0f);
        w.emplace<component::fire_state>(e, false, 1.0f);
        w.emplace<component::physics>(e, nullptr);
        player ? w.emplace<component::player>(e) : w.emplace<component::enemy>(e);
        return e;
    }

    entity::id create_bullet(entity::world& w) {
        auto const e = w.create();
        w.emplace<component::bullet>(e);
        w.emplace<component::skin>(e, nullptr);
        w.emplace<component::move_state>(e, 0.0f);
        w.emplace<component::lifetime>(e);
        w.emplace<component::physics>(e, nullptr);
        w.emplace<component::owner_info>(e, entt::null);
        return e;
    }

    entity::id create_physics_layer(entity::world& w) {
        auto const e = w.create();
        w.emplace<component::physics_layer>(e);
        w.emplace<component::physics_layer_visible>(e, true);
        w.emplace<component::physics_collision_handler>(e, nullptr, nullptr);
        return e;
    }
}