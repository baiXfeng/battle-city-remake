//
// Created by baifeng on 2021/11/14.
//

#include "systems.h"
#include "components.h"
#include "factory.h"
#include "common/widget.h"
#include "common/loadres.h"
#include "angles/angles.hpp"
#include "src/view.h"

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

    void ObjectAI_System::update(Context& c) {
        auto view = c.reg.view<component::enemy, component::move_state>();
        for (auto e : view) {

        }
    }

    //=====================================================================================

    void ObjectMoveSystem::update(Context& c) {
        auto view = c.reg.view<component::move_state, component::skin, component::physics>();
        for (auto e : view) {
            auto& state = view.get<component::move_state>(e);
            auto speed = state.speed;

            if (auto brake = c.reg.try_get<component::tank_brake>(e); brake and brake->value) {
                speed = {0.0f, 0.0f};
            }

            if (auto& phys = view.get<component::physics>(e); phys.body) {
                b2BodySugar s(phys.body);
                auto& skin = view.get<component::skin>(e);
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
            if (lifetime.value >= 1.0f) {
                c.reg.emplace_or_replace<component::killed>(e);
            }
        }
    }

    //=====================================================================================

    void ObjectCleanSystem::update(Context& c) {
        // 释放子弹
        if (auto view = c.reg.view<component::killed, component::bullet>(); true) {
            for (auto e: view) {
                bullet::destroy(c, e);
            }
        }

        // 释放坦克
        if (auto view = c.reg.view<component::killed, component::tank>(); true) {
            for (auto e : view) {
                tank::destroy(c, e);
            }
        }

        // 通常释放
        if (auto view = c.reg.view<component::killed>(); true) {
            for (auto e: view) {
                c.reg.destroy(e);
            }
        }
    }

    //=====================================================================================

    void PhysicsSystem::update(Context& c) {
        b2WorldSugar::update(c.physics(), c.delta());
        auto view = c.reg.view<component::skin, component::physics>();
        for (auto e : view) {
            if (auto& phys = view.get<component::physics>(e); phys.body) {
                b2BodySugar s(phys.body);
                auto& skin = view.get<component::skin>(e);
                skin.view->setPosition(s.getPixelPosition());
                skin.view->children()[0]->setRotation(s.getPixelAngle());
            }
        }
    }

    //=====================================================================================

    void onBulletHit(event::EntityPhysicsContact const& e) {
        auto target = (entity::id)e.target->GetBody()->GetUserData().pointer;
        if (e.c->reg.any_of<component::tank>(target)) {
            onBulletHitTank(e);
        } else if (e.c->reg.any_of<component::bullet>(target)) {
            onBulletHitBullet(e);
        } else if (e.c->reg.any_of<component::wall>(target)) {
            onBulletHitWall(e);
        }
    }

    //=====================================================================================

    void onBulletHitTank(event::EntityPhysicsContact const& e) {

        auto sender = (entity::id)e.sender->GetBody()->GetUserData().pointer;
        auto target = (entity::id)e.target->GetBody()->GetUserData().pointer;

        // bullet explosion
        if (auto skin = e.c->reg.try_get<component::skin>(sender); skin) {
            auto view = Widget::New<BulletExplosionView>();
            view->setAnchor(0.5f, 0.5f);
            view->setPosition( skin->view->position() );
            view->fast_to<BulletExplosionView>()->play();
            e.c->rootView->addChild(view);
        }

        // tank explosion
        if (auto skin = e.c->reg.try_get<component::skin>(target); skin) {
            auto view = Widget::New<BigExplosionView>();
            view->setAnchor(0.5f, 0.5f);
            view->setPosition( skin->view->position() );
            view->fast_to<BigExplosionView>()->play();
            e.c->rootView->addChild(view);
        }

        // kill entity
        e.c->reg.emplace_or_replace<component::killed>(sender);
        e.c->reg.emplace_or_replace<component::killed>(target);

        for (auto fixture = e.sender->GetBody()->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
            auto entity = (entity::id)fixture->GetUserData().pointer;
            e.c->reg.emplace_or_replace<component::killed>(entity);
        }
        for (auto fixture = e.target->GetBody()->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
            auto entity = (entity::id)fixture->GetUserData().pointer;
            e.c->reg.emplace_or_replace<component::killed>(entity);
        }
    }

    void onBulletHitBullet(event::EntityPhysicsContact const& e) {

    }

    void onBulletHitWall(event::EntityPhysicsContact const& e) {

        auto sender = (entity::id)e.sender->GetBody()->GetUserData().pointer;

        // bullet explosion
        if (auto skin = e.c->reg.try_get<component::skin>(sender); skin) {
            auto view = Widget::New<BulletExplosionView>();
            view->setAnchor(0.5f, 0.5f);
            view->setPosition( skin->view->position() );
            view->fast_to<BulletExplosionView>()->play();
            e.c->rootView->addChild(view);
        }

        // kill entity
        e.c->reg.emplace_or_replace<component::killed>(sender);

        for (auto fixture = e.sender->GetBody()->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
            auto entity = (entity::id)fixture->GetUserData().pointer;
            e.c->reg.emplace_or_replace<component::killed>(entity);
        }
    }

    //=====================================================================================

    ContactFilter::ContactFilter(Context& c):_c(&c) {}

    bool ContactFilter::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) {

        auto& filterA = fixtureA->GetFilterData();
        auto& filterB = fixtureB->GetFilterData();

        if ((filterA.maskBits & filterB.categoryBits) != 0 or (filterA.categoryBits & filterB.maskBits) != 0) {

            auto bodyEntityA = (entity::id)fixtureA->GetBody()->GetUserData().pointer;
            auto bodyEntityB = (entity::id)fixtureB->GetBody()->GetUserData().pointer;

            if (auto owner = _c->reg.try_get<component::owner_info>(bodyEntityA); owner) {
                if (owner->entity == bodyEntityB) {
                    return false;
                }
            }

            if (auto owner = _c->reg.try_get<component::owner_info>(bodyEntityB); owner) {
                if (owner->entity == bodyEntityA) {
                    return false;
                }
            }

            auto fixtureEntityA = (entity::id)fixtureA->GetUserData().pointer;
            if (auto visible = _c->reg.try_get<component::physics_layer_visible>(fixtureEntityA); visible) {
                if (!visible->value) {
                    return false;
                }
            }

            auto fixtureEntityB = (entity::id)fixtureB->GetUserData().pointer;
            if (auto visible = _c->reg.try_get<component::physics_layer_visible>(fixtureEntityB); visible) {
                if (!visible->value) {
                    return false;
                }
            }

            return true;
        }

        return false;
    }

    //=====================================================================================

    ContactListener::ContactListener(Context& c):_c(&c) {}

    void ContactListener::BeginContact(b2Contact* contact) {
        auto fixtureA = contact->GetFixtureA();
        auto fixtureB = contact->GetFixtureB();

        if (fixtureB->GetFilterData().maskBits & fixtureA->GetFilterData().categoryBits) {
            auto temp = fixtureA;
            fixtureA = fixtureB;
            fixtureB = temp;
        }
        if (fixtureA->GetFilterData().maskBits & fixtureB->GetFilterData().categoryBits) {
            auto entity = (entity::id)fixtureA->GetUserData().pointer;
            if (auto handler = _c->reg.try_get<component::physics_collision_handler>(entity); handler and handler->begin) {
                handler->begin({_c, contact, fixtureA, fixtureB});
            }
        }
    }

    void ContactListener::EndContact(b2Contact* contact) {
        auto fixtureA = contact->GetFixtureA();
        auto fixtureB = contact->GetFixtureB();

        if (fixtureB->GetFilterData().maskBits & fixtureA->GetFilterData().categoryBits) {
            auto temp = fixtureA;
            fixtureA = fixtureB;
            fixtureB = temp;
        }
        if (fixtureA->GetFilterData().maskBits & fixtureB->GetFilterData().categoryBits) {
            auto entity = (entity::id)fixtureA->GetUserData().pointer;
            if (auto handler = _c->reg.try_get<component::physics_collision_handler>(entity); handler and handler->end) {
                handler->end({_c, contact, fixtureA, fixtureB});
            }
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

            auto const entity = factory::create_tank(c.reg, t == Party::PLAYER);
            auto& skin = c.reg.get<component::skin>(entity);
            skin.view = tankView.get();

            auto& phys = c.reg.get<component::physics>(entity);
            phys.body = b2BodySugar::CreateBody(c.physics(), b2_dynamicBody);
            phys.body->GetUserData().pointer = (uintptr_t)entity;

            b2BodySugar s(phys.body);
            if (auto shape = s.addCircleShape(image->size().x * 0.45f); shape) {
                b2Filter filter;
                filter.categoryBits = int(CollisionBit::DAMAGE_BOX);
                filter.maskBits = 0;
                shape->SetFilterData(filter);

                auto layer = factory::create_physics_layer(c.reg);
                shape->GetUserData().pointer = (uintptr_t)layer;
            }
            if (auto shape = s.addBoxShape((image->size() * 0.8f).to<int>()); shape) {
                b2Filter filter;
                filter.categoryBits = int(CollisionBit::TANK_BODY);
                filter.maskBits = int(CollisionBit::TANK_BODY);
                shape->SetFilterData(filter);

                auto layer = factory::create_physics_layer(c.reg);
                shape->GetUserData().pointer = (uintptr_t)layer;
            }
            if (auto shape = s.addCircleShape(image->size().x * 3.0f); shape) {
                b2Filter filter;
                filter.categoryBits = 0;
                filter.maskBits = int(CollisionBit::TANK_BODY);
                shape->SetFilterData(filter);
                shape->SetSensor(true);

                auto layer = factory::create_physics_layer(c.reg);
                shape->GetUserData().pointer = (uintptr_t)layer;
            }
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
            c.reg.destroy(e);
        }
    }

    //=====================================================================================

    namespace bullet {

        void create(Context& c, entity::id sender, mge::Vector2f const& pos, float rotation) {
            auto const entity = factory::create_bullet(c.reg);
            auto& skin = c.reg.get<component::skin>(entity);

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

            auto& move_state = c.reg.get<component::move_state>(entity);
            move_state.speed.reset(x, y);
            move_state.speed *= 600.0f;
            move_state.rotation = rotation;

            auto& owner_info = c.reg.get<component::owner_info>(entity);
            owner_info.entity = sender;

            auto& phys = c.reg.get<component::physics>(entity);
            phys.body = b2BodySugar::CreateBody(c.physics(), b2_dynamicBody);
            phys.body->GetUserData().pointer = (uintptr_t)entity;

            b2BodySugar s(phys.body);
            if (auto shape = s.addCircleShape(image->size().x * 0.5f); shape) {
                b2Filter filter;
                filter.categoryBits = 0;
                filter.maskBits = int(CollisionBit::DAMAGE_BOX);
                shape->SetFilterData(filter);

                auto layer = factory::create_physics_layer(c.reg);
                shape->GetUserData().pointer = (uintptr_t)layer;
                auto& handler = c.reg.get<component::physics_collision_handler>(layer);
                handler.begin = &onBulletHit;
            }
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
            c.reg.destroy(e);
        }
    }
}