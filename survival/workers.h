//
// Created by baifeng on 2022/1/16.
//

#ifndef SDL2_UI_WORKERS_H
#define SDL2_UI_WORKERS_H

#include "context.h"
#include "events.h"
#include <memory>

namespace entity {

    class Worker {
    public:
        typedef std::shared_ptr<Worker> Ptr;
    public:
        virtual ~Worker() {}
    };

    class PlayerInputWorker : public Worker {
        enum {
            UP = 0x1,
            DOWN = 0x2,
            LEFT = 0x4,
            RIGHT = 0x8,
            LEFT_UP = LEFT | UP,
            LEFT_DOWN = LEFT | DOWN,
            RIGHT_UP = RIGHT | UP,
            RIGHT_DOWN = RIGHT | DOWN,
        };
    public:
        PlayerInputWorker(Context& c);
        ~PlayerInputWorker();
    private:
        void onButtonDown(event::GamepadDown const& e);
        void onButtonUp(event::GamepadUp const& e);
        void onJoyMotion(event::GamepadAxisMotion const& e);
        void onKeyMask(uint32_t key);
    private:
        uint32_t _keymask;
        entt::dispatcher* _dispatcher;
        entity::world* _reg;
    };

    class CollisionWorker : public Worker {
    public:
        CollisionWorker(Context& c);
        ~CollisionWorker();
    private:
        void onEntityBeginTouch(event::EntityBeginTouch const& e);
        void onEntityEndTouch(event::EntityEndTouch const& e);
    private:
        void onBulletHitTank(component::entity_info* bullet, component::entity_info* tank);
    private:
        Context* _c;
    };
}

#endif //SDL2_UI_WORKERS_H
