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

    class PhysicsSystem : public System {
        void update(Context& c) override;
    };

    class ContactFilter : public b2ContactFilter {
    public:
        ContactFilter(Context& c);
    private:
        bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) override;
        Context* _c;
    };

    class ContactListener : public b2ContactListener {
    public:
        ContactListener(Context& c);
    private:
        void BeginContact(b2Contact* contact) override;
        void EndContact(b2Contact* contact) override;
        void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
        void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;
        Context* _c;
    };

    enum Party {
        PLAYER = 0,
        ENEMY,
    };

    namespace tank {
        void create(Context& c, Party t, mge::Vector2f const& pos);
        void destroy(Context& c, entity::id e);
    }

    namespace bullet {
        void create(Context& c, entity::id sender, mge::Vector2f const& pos, float rotation);
        void destroy(Context& c, entity::id e);
    }

}

#endif //SDL2_UI_SYSTEMS_H
