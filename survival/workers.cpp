//
// Created by baifeng on 2022/1/16.
//

#include "workers.h"
#include "common/gamepad.h"
#include "common/widget.h"
#include "common/log.h"
#include "components.h"
#include "angles/angles.hpp"
#include "src/view.h"

namespace entity {

    using namespace mge;

    typedef GamePadListener::KeyCode KeyCode;

    //=====================================================================================

    PlayerInputWorker::PlayerInputWorker(Context& c): _dispatcher(&c.dispatcher), _reg(&c.reg), _keymask(0) {
        _dispatcher->sink<event::GamepadDown>().connect<&PlayerInputWorker::onButtonDown>(this);
        _dispatcher->sink<event::GamepadUp>().connect<&PlayerInputWorker::onButtonUp>(this);
        _dispatcher->sink<event::GamepadAxisMotion>().connect<&PlayerInputWorker::onJoyMotion>(this);
    }

    PlayerInputWorker::~PlayerInputWorker() {
        _dispatcher->sink<event::GamepadDown>().disconnect(this);
        _dispatcher->sink<event::GamepadUp>().disconnect(this);
        _dispatcher->sink<event::GamepadAxisMotion>().disconnect(this);
    }

    void PlayerInputWorker::onButtonDown(event::GamepadDown const &e) {
        //printf("key down = %d\n", e.key);
        if (e.key == KeyCode::R1) {
            // player fire
            auto view = _reg->view<component::player, component::fire_state>();
            for (auto e : view) {
                auto& fire_state = view.get<component::fire_state>(e);
                fire_state.fire = true;
            }
        } else if (e.key == KeyCode::X) {
            auto view = _reg->view<component::player, component::tank_brake>();
            for (auto e : view) {
                auto& brake = view.get<component::tank_brake>(e);
                brake.value = true;
            }
        } else if (e.key >= KeyCode::UP and e.key <= KeyCode::RIGHT) {
            int mask[4] = {UP, DOWN, LEFT, RIGHT};
            _keymask |= mask[e.key-1];
            this->onKeyMask(_keymask);
        }
    }

    void PlayerInputWorker::onButtonUp(event::GamepadUp const &e) {
        //printf("key up = %d\n", e.key);
        if (e.key == KeyCode::R1) {
            // player fire
            auto view = _reg->view<component::player, component::fire_state>();
            for (auto e : view) {
                auto& fire_state = view.get<component::fire_state>(e);
                fire_state.fire = false;
            }
        } else if (e.key == KeyCode::X) {
            auto view = _reg->view<component::player, component::tank_brake>();
            for (auto e : view) {
                auto& brake = view.get<component::tank_brake>(e);
                brake.value = false;
            }
        } else if (e.key >= KeyCode::UP and e.key <= KeyCode::RIGHT) {
            int mask[4] = {UP, DOWN, LEFT, RIGHT};
            _keymask &= ~mask[e.key-1];
            this->onKeyMask(_keymask);
        }
    }

    void PlayerInputWorker::onJoyMotion(event::GamepadAxisMotion const& e) {
        if (e.joy_id != 0) {
            return;
        }
        auto angle = Degrees<float>::atan(e.x, -e.y);
        auto view = _reg->view<component::player, component::skin, component::move_state>();
        for (auto entity : view) {
            auto& skin = view.get<component::skin>(entity);
            auto& state = view.get<component::move_state>(entity);
            if (abs(e.x) <= 80 and abs(e.y) <= 80) {
                state.speed.reset(0, 0);
                continue;
            }
            state.speed = Vector2f{e.x, e.y}.normalized() * 300;
            state.rotation = angle.getScalarValue();
        }
    }

    void PlayerInputWorker::onKeyMask(uint32_t key) {
        event::GamepadAxisMotion e{0, 0, 0};
        switch (key) {
            case UP:
                e.y = -128;
                break;
            case DOWN:
                e.y = 128;
                break;
            case LEFT:
                e.x = -128;
                break;
            case RIGHT:
                e.x = 128;
                break;
            case LEFT_UP:
                e.x = -128;
                e.y = -128;
                break;
            case LEFT_DOWN:
                e.x = -128;
                e.y = 128;
                break;
            case RIGHT_UP:
                e.x = 128;
                e.y = -128;
                break;
            case RIGHT_DOWN:
                e.x = 128;
                e.y = 128;
                break;
            default:
                break;
        }
        onJoyMotion(e);
    }

}