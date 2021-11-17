//
// Created by baifeng on 2021/11/14.
//

#ifndef SDL2_UI_SYSTEMS_H
#define SDL2_UI_SYSTEMS_H

#include "context.h"
#include "events.h"
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

    class InputSystem : public System {
    public:
        InputSystem(Context& c);
        ~InputSystem();
    private:
        void update(Context& c) override;
        void onButtonDown(event::GamepadDown const& e);
        void onButtonUp(event::GamepadUp const& e);
    private:
        entt::dispatcher* _dispatcher;
        entity::world* _world;
    };

    class TankFireSystem : public System {
        void update(Context& c) override;
    };
}

#endif //SDL2_UI_SYSTEMS_H
