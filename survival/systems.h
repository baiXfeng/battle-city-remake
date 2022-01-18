//
// Created by baifeng on 2021/11/14.
//

#ifndef SDL2_UI_SYSTEMS_H
#define SDL2_UI_SYSTEMS_H

#include "context.h"
#include "events.h"
#include "common/vector2.h"
#include <memory>
#include <vector>

namespace entity {

    class System {
    public:
        typedef std::shared_ptr<System> Ptr;
    public:
        virtual ~System() {}
        virtual void update(Context& c) = 0;
    };

    class SequenceSystem : public System {
    public:
        typedef std::vector<Ptr> Array;
    public:
        SequenceSystem();
        SequenceSystem(Array const& arr);
    protected:
        void update(Context& c) override;
        void add(Ptr const& sys);
    protected:
        Array _systems;
    };

    class ObjectMoveSystem : public System {
        void update(Context& c) override;
    };

    class TankFireSystem : public System {
        void update(Context& c) override;
    };

    class ObjectCheckOutSystem : public System {
        void update(Context& c) override;
    };

    class ObjectCleanSystem : public System {
        void update(Context& c) override;
    };

    namespace tank {
        enum Type {
            PLAYER = 0,
            ENEMY,
        };
        void create(Context& c, Type t, mge::Vector2f const& pos);
        void destroy(Context& c, entity::id e);
    }

}

#endif //SDL2_UI_SYSTEMS_H
