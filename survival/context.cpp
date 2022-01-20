//
// Created by baifeng on 2022/1/19.
//

#include "context.h"

namespace entity {

    Context::Context():_delta(0.0f), _physics(mge::b2WorldSugar::CreateWorld()), _b2draw(new mge::PhysicDrawner) {
        _physics->SetDebugDraw(_b2draw.get());
    }

    Context::~Context() {
        delete _physics;
    }

    float Context::delta() const {
        return _delta;
    }

    void Context::delta(float const& v) {
        _delta = v;
    }

    b2World* Context::physics() const {
        return _physics;
    }
}