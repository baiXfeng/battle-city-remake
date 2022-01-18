//
// Created by baifeng on 2021/11/14.
//

#include "factory.h"
#include "components.h"

namespace factory {

    entity::id create_tank(entity::world& w, bool player) {
        auto const e = w.create();
        w.emplace<component::tank_brake>(e, false);
        w.emplace<component::skin>(e, nullptr);
        w.emplace<component::move_speed>(e);
        w.emplace<component::fire_state>(e, false, 1.0f);
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
        w.emplace<component::move_speed>(e);
        w.emplace<component::lifetime>(e);
        return e;
    }
}