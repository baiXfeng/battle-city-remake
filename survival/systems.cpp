//
// Created by baifeng on 2021/11/14.
//

#include "systems.h"
#include "common/gamepad.h"
#include "common/widget.h"
#include "components.h"

namespace entity {

    using namespace mge;

    //=====================================================================================

    SequenceSystem::SequenceSystem() {}

    SequenceSystem::SequenceSystem(Array const &arr) : _systems(arr) {

    }

    void SequenceSystem::update(Context &c) {
        for (auto &sys: _systems) {
            sys->update(c);
        }
    }

    void SequenceSystem::add(Ptr const &sys) {
        _systems.push_back(sys);
    }

    //=====================================================================================

    float _angle = 0.0f;
    float _bodyangle = 0.0f;

    InputSystem::InputSystem(Context& c):_dispatcher(&c.dispatcher), _world(&c.world) {
        _dispatcher->sink<event::GamepadDown>().connect<&InputSystem::onButtonDown>(this);
        _dispatcher->sink<event::GamepadUp>().connect<&InputSystem::onButtonUp>(this);
    }

    InputSystem::~InputSystem() {
        _dispatcher->sink<event::GamepadDown>().disconnect(this);
        _dispatcher->sink<event::GamepadUp>().disconnect(this);
    }

    void InputSystem::update(Context& c) {
        auto& reg = *_world;
        auto view = reg.view<component::player, component::transform, component::weapon, component::skin>();
        for (auto&& [player, trans, weapon, skin] : view.each()) {
            weapon.rotation += _angle * c.delta;
            skin.weapon->setRotation(weapon.rotation);
            trans.rotation += _bodyangle * c.delta;
            skin.body->setRotation(trans.rotation);
        }
    }

    void InputSystem::onButtonDown(event::GamepadDown const &e) {
        //printf("key down = %d\n", e.key);
        if (e.key == GamePad::KeyCode::LEFT) {
            _angle = -100.0f;
        } else if (e.key == GamePad::KeyCode::RIGHT) {
            _angle = 100.0f;
        } else if (e.key == GamePad::KeyCode::UP) {
            _bodyangle = -100.0f;
        } else if (e.key == GamePad::KeyCode::DOWN) {
            _bodyangle = 100.0f;
        }
    }

    void InputSystem::onButtonUp(event::GamepadUp const &e) {
        //printf("key up = %d\n", e.key);
        if (e.key == GamePad::KeyCode::LEFT) {
            _angle = 0.0f;
        } else if (e.key == GamePad::KeyCode::RIGHT) {
            _angle = 0.0f;
        } else if (e.key == GamePad::KeyCode::UP) {
            _bodyangle = 0.0f;
        } else if (e.key == GamePad::KeyCode::DOWN) {
            _bodyangle = 0.0f;
        }
    }

    //=====================================================================================

    void TankFireSystem::update(Context& c) {

    }

}