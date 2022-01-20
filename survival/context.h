//
// Created by baifeng on 2021/11/14.
//

#ifndef SDL2_UI_CONTEXT_H
#define SDL2_UI_CONTEXT_H

#include "entity.h"
#include "components.h"
#include "common/physics.h"

namespace entity {
    class Context {
        typedef std::map<entity::id, component::entity_info> InfoPool;
    public:
        entt::dispatcher dispatcher;
        entity::world reg;
        entity::view rootView;
        InfoPool entity_info_pool;  // for box2d physics system
    public:
        Context();
        ~Context();
        float delta() const;
        void delta(float const& v);
        b2World* physics() const;
    private:
        float _delta;
        b2World* _physics;
        std::shared_ptr<b2Draw> _b2draw;
    };
}

#endif //SDL2_UI_CONTEXT_H
