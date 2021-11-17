//
// Created by baifeng on 2021/11/14.
//

#include "factory.h"
#include "components.h"

namespace factory {

    entity::id create_tank(entity::world& w, bool player) {
        auto const e = w.create();
        w.emplace<component::transform>(e);
        w.emplace<component::moveable>(e);
        w.emplace<component::weapon>(e);
        w.emplace<component::skin>(e, nullptr, nullptr, nullptr);
        if (player) {
            w.emplace<component::player>(e);
        } else {
            w.emplace<component::enemy>(e);
        }
        return e;
    }
}