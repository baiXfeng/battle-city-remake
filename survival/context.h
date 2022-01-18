//
// Created by baifeng on 2021/11/14.
//

#ifndef SDL2_UI_CONTEXT_H
#define SDL2_UI_CONTEXT_H

#include "entity.h"

namespace entity {
    class Context {
    public:
        Context():_delta(0.0f) {}
        inline float delta() const {
            return _delta;
        }
        void delta(float const& v) {
            _delta = v;
        }
        entt::dispatcher dispatcher;
        entity::world reg;
        entity::view rootView;
    private:
        float _delta;
    };
}

#endif //SDL2_UI_CONTEXT_H
