//
// Created by baifeng on 2021/11/14.
//

#include "systems.h"
#include "components.h"
#include "factory.h"
#include "common/widget.h"
#include "common/loadres.h"
#include "angles/angles.hpp"

namespace entity {

    using namespace mge;

    //=====================================================================================

    SequenceSystem::SequenceSystem() {}

    SequenceSystem::SequenceSystem(Array const &arr) : _systems(arr) {

    }

    void SequenceSystem::update(Context &c) {
        for (auto& sys: _systems) {
            sys->update(c);
        }
    }

    void SequenceSystem::add(Ptr const &sys) {
        _systems.push_back(sys);
    }

    //=====================================================================================

    void ObjectMoveSystem::update(Context& c) {
        auto view = c.reg.view<component::move_speed, component::skin>();
        for (auto e : view) {
            auto& move = view.get<component::move_speed>(e);
            auto& skin = view.get<component::skin>(e);

            if (c.reg.any_of<component::tank_brake>(e)) {
                auto& brake = c.reg.get<component::tank_brake>(e);
                if (brake.value) {
                    continue;
                }
            }

            skin.view->setPosition(move.value * c.delta() + skin.view->position() );
        }
    }

    //=====================================================================================

    void TankFireSystem::update(Context& c) {
        auto view = c.reg.view<component::fire_state, component::move_speed, component::skin>();
        for (auto e : view) {
            auto& tank_skin = view.get<component::skin>(e);
            auto& tank_move_speed = view.get<component::move_speed>(e);
            auto& fire_state = view.get<component::fire_state>(e);
            if ((fire_state.cooldown += c.delta()) >= 0.3f) {
                fire_state.cooldown = 0.3f;
                if (!fire_state.fire) {
                    continue;
                }
                fire_state.cooldown = 0.0f;

                // tank fire
                auto bullet = factory::create_bullet(c.reg);
                auto& bullet_skin = c.reg.get<component::skin>(bullet);
                auto& bullet_move_speed = c.reg.get<component::move_speed>(bullet);

                auto view = Widget::New<mge::Widget>();
                view->setPosition( tank_skin.view->position() );
                c.rootView->addChild(view);
                bullet_skin.view = view.get();

                auto image = Widget::New<mge::ImageWidget>(res::load_texture("assets/images/bullet_up.png"));
                image->setAnchor(0.5f, 0.5f);
                image->setRotation( tank_skin.view->children()[0]->rotation() );
                view->addChild(image);

                float angle = image->rotation() - 90.0f;
                Degrees<float> d(angle);
                auto rad = d.toRadians();
                auto x = rad.cos();
                auto y = rad.sin();
                bullet_move_speed.value.reset(x, y);
                bullet_move_speed.value *= 600.0f;
            }
        }
    }

    //=====================================================================================

    void ObjectCheckOutSystem::update(Context& c) {
        auto view = c.reg.view<component::lifetime>();
        for (auto e : view) {
            auto& lifetime = view.get<component::lifetime>(e);
            lifetime.value += c.delta();
            if (lifetime.value >= 0.25f) {
                c.reg.emplace_or_replace<component::killed>(e);
            }
        }
    }

    //=====================================================================================

    void ObjectCleanSystem::update(Context& c) {
        // 释放子弹
        auto view = c.reg.view<component::killed, component::bullet, component::skin>();
        for (auto e : view) {
            tank::destroy(c, e);
        }
    }

    //=====================================================================================
    namespace tank {

        using namespace mge;

        void create(Context& c, Type t, mge::Vector2f const& pos) {
            auto tankView = Widget::New<mge::Widget>();
            tankView->setPosition(pos);
            c.rootView->addChild(tankView);

            auto image = Widget::New<mge::ImageWidget>(mge::res::load_texture("assets/images/tank_player1_up_c0_t1_s1.png"));
            image->setAnchor(0.5f, 0.5f);
            image->setSize(48, 48);
            tankView->addChild(image);

            auto tank = factory::create_tank(c.reg, true);
            auto& skin = c.reg.get<component::skin>(tank);
            skin.view = tankView.get();
        }

        void destroy(Context& c, entity::id e) {
            auto& skin = c.reg.get<component::skin>(e);
            if (skin.view) {
                skin.view->removeFromParent();
            }
            c.reg.destroy(e);
        }
    }
}