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
        if (player) {
            w.emplace<component::player>(e);
        } else {
            w.emplace<component::enemy>(e);
        }
        return e;
    }

    entity::id create_bullet(entity::world& w) {
        auto const e = w.create();
        w.emplace<component::bullet>(e);
        w.emplace<component::skin>(e, nullptr);
        w.emplace<component::move_state>(e, 0.0f);
        w.emplace<component::lifetime>(e);
        w.emplace<component::physics>(e, nullptr);
        return e;
    }
}