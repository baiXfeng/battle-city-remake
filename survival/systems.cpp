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
        auto view = c.reg.view<component::move_state, component::skin, component::physics>();
        for (auto e : view) {
            auto& state = view.get<component::move_state>(e);
            auto& skin = view.get<component::skin>(e);
            auto speed = state.speed;

            if (c.reg.any_of<component::tank_brake>(e)) {
                auto& brake = c.reg.get<component::tank_brake>(e);
                if (brake.value) {
                    //continue;
                    speed = {0.0f, 0.0f};
                }
            }

            if (auto& phys = view.get<component::physics>(e); phys.body) {
                b2BodySugar s(phys.body);
                s.setTransform(skin.view->position(), state.rotation);
                s.setLinearVelocity(speed);
            }
        }
    }

    //=====================================================================================

    void TankFireSystem::update(Context& c) {
        auto view = c.reg.view<component::tank, component::fire_state, component::skin>();
        for (auto e : view) {
            auto& tank_skin = view.get<component::skin>(e);
            auto& fire_state = view.get<component::fire_state>(e);
            if ((fire_state.cooldown += c.delta()) >= 0.3f) {
                fire_state.cooldown = 0.3f;
                if (!fire_state.fire) {
                    continue;
                }
                fire_state.cooldown = 0.0f;

                // tank fire
                bullet::create(c, e, tank_skin.view->position(), tank_skin.view->children()[0]->rotation());
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
        {
            auto view = c.reg.view<component::killed, component::bullet, component::skin>();
            for (auto e: view) {
                bullet::destroy(c, e);
            }
        }

        // 释放坦克
        {
            auto view = c.reg.view<component::killed, component::tank, component::skin>();
            for (auto e : view) {
                tank::destroy(c, e);
            }
        }
    }

    //=====================================================================================

    void PhysicsSystem::update(Context& c) {
        b2WorldSugar::update(c.physics(), c.delta());
        auto view = c.reg.view<component::skin, component::physics>();
        for (auto e : view) {
            auto& skin = view.get<component::skin>(e);
            if (auto& phys = view.get<component::physics>(e); phys.body) {
                b2BodySugar s(phys.body);
                skin.view->setPosition(s.getPixelPosition());
                skin.view->children()[0]->setRotation(s.getPixelAngle());
            }
        }
    }

    ContactFilter::ContactFilter(Context& c):_c(&c) {}

    bool ContactFilter::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) {
        auto bodyA = fixtureA->GetBody();
        auto bodyB = fixtureB->GetBody();
        if (bodyA->GetUserData().pointer and bodyB->GetUserData().pointer) {
            auto infoA = (component::entity_info*)bodyA->GetUserData().pointer;
            auto infoB = (component::entity_info*)bodyB->GetUserData().pointer;
            if (infoA->owner == infoB->sender or infoA->sender == infoB->owner) {
                // 子弹不与发射者碰撞
                return false;
            } else if (infoA->type == infoB->type and infoA->type == component::entity_type::BULLET) {
                // 子弹不与子弹碰撞
                return false;
            }
        }
        return b2ContactFilter::ShouldCollide(fixtureA, fixtureB);
    }

    ContactListener::ContactListener(Context& c):_c(&c) {}

    void ContactListener::BeginContact(b2Contact* contact) {
        auto bodyA = contact->GetFixtureA()->GetBody();
        auto bodyB = contact->GetFixtureB()->GetBody();
        if (bodyA->GetUserData().pointer and bodyB->GetUserData().pointer) {
            auto infoA = (component::entity_info *) bodyA->GetUserData().pointer;
            auto infoB = (component::entity_info *) bodyB->GetUserData().pointer;
            _c->dispatcher.trigger(event::EntityBeginTouch{infoA, infoB});
        }
    }

    void ContactListener::EndContact(b2Contact* contact) {
        auto bodyA = contact->GetFixtureA()->GetBody();
        auto bodyB = contact->GetFixtureB()->GetBody();
        if (bodyA->GetUserData().pointer and bodyB->GetUserData().pointer) {
            auto infoA = (component::entity_info *) bodyA->GetUserData().pointer;
            auto infoB = (component::entity_info *) bodyB->GetUserData().pointer;
            _c->dispatcher.trigger(event::EntityEndTouch{infoA, infoB});
        }
    }

    void ContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {

    }

    void ContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {

    }

    //=====================================================================================

    namespace tank {

        using namespace mge;

        void create(Context& c, Party t, mge::Vector2f const& pos) {
            auto tankView = Widget::New<mge::Widget>();
            tankView->setPosition(pos);
            c.rootView->addChild(tankView);

            auto image = Widget::New<mge::ImageWidget>(mge::res::load_texture("assets/images/tank_player1_up_c0_t1_s1.png"));
            image->setAnchor(0.5f, 0.6f);
            image->setSize(48, 48);
            tankView->addChild(image);

            auto tank = factory::create_tank(c.reg, t == PLAYER);
            auto& skin = c.reg.get<component::skin>(tank);
            skin.view = tankView.get();

            auto& entity_info = c.entity_info_pool[tank] = {tank, entt::null, component::entity_type::TANK};
            auto& phys = c.reg.get<component::physics>(tank);
            phys.body = b2BodySugar::CreateBody(c.physics(), b2_dynamicBody);
            phys.body->GetUserData().pointer = (uintptr_t)&entity_info;

            b2BodySugar s(phys.body);
            s.addCircleShape(image->size().x * 0.45f);
            s.setTransform(skin.view->position());
            s.enableRotation(false);
        }

        void destroy(Context& c, entity::id e) {
            if (auto& skin = c.reg.get<component::skin>(e); skin.view) {
                skin.view->removeFromParent();
            }
            if (auto& phys = c.reg.get<component::physics>(e); phys.body) {
                c.physics()->DestroyBody(phys.body);
            }
            c.entity_info_pool.erase(e);
            c.reg.destroy(e);
        }
    }

    //=====================================================================================

    namespace bullet {

        void create(Context& c, entity::id sender, mge::Vector2f const& pos, float rotation) {
            auto bullet = factory::create_bullet(c.reg);
            auto& skin = c.reg.get<component::skin>(bullet);
            auto& move_state = c.reg.get<component::move_state>(bullet);
            auto& entity_info = c.entity_info_pool[bullet] = {bullet, sender, component::entity_type::BULLET};

            auto view = Widget::New<mge::Widget>();
            view->setPosition( pos );
            c.rootView->addChild(view);
            skin.view = view.get();

            auto image = Widget::New<mge::ImageWidget>(res::load_texture("assets/images/bullet_up.png"));
            image->setAnchor(0.5f, 0.5f);
            image->setRotation( rotation );
            view->addChild(image);

            float angle = image->rotation() - 90.0f;
            Degrees<float> d(angle);
            auto rad = d.toRadians();
            auto x = rad.cos();
            auto y = rad.sin();
            move_state.speed.reset(x, y);
            move_state.speed *= 600.0f;
            move_state.rotation = rotation;

            auto& phys = c.reg.get<component::physics>(bullet);
            phys.body = b2BodySugar::CreateBody(c.physics(), b2_dynamicBody);
            phys.body->GetUserData().pointer = (uintptr_t)&entity_info;

            b2BodySugar s(phys.body);
            s.addCircleShape(image->size().x * 0.5f);
            s.setTransform(skin.view->position());
            s.enableRotation(false);
        }

        void destroy(Context& c, entity::id e) {
            if (auto& skin = c.reg.get<component::skin>(e); skin.view) {
                skin.view->removeFromParent();
            }
            if (auto& phys = c.reg.get<component::physics>(e); phys.body) {
                c.physics()->DestroyBody(phys.body);
            }
            c.entity_info_pool.erase(e);
            c.reg.destroy(e);
        }
    }
}