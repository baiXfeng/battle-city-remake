//
// Created by baifeng on 2021/11/13.
//

#include "world.h"
#include "factory.h"
#include "systems.h"
#include "workers.h"
#include "object_view.h"
#include "common/loadres.h"
#include "box2d/box2d.h"

SurvivalView::SurvivalView(): _filter(new entity::ContactFilter(_c)), _listener(new entity::ContactListener(_c)) {

    _c.rootView = this;
    _c.physics()->SetContactFilter(_filter.get());
    _c.physics()->SetContactListener(_listener.get());

    entity::tank::create(_c, entity::PLAYER, size() * 0.5f);
    entity::tank::create(_c, entity::ENEMY, {size().x * 0.25f, size().y * 0.25f});
    entity::tank::create(_c, entity::ENEMY, {size().x * 0.75f, size().y * 0.25f});
    entity::tank::create(_c, entity::ENEMY, {size().x * 0.25f, size().y * 0.75f});
    entity::tank::create(_c, entity::ENEMY, {size().x * 0.75f, size().y * 0.75f});

    _worker.push_back(entity::Worker::Ptr(new entity::PlayerInputWorker(_c)));
    _worker.push_back(entity::Worker::Ptr(new entity::CollisionWorker(_c)));

    auto fire_system = entity::System::Ptr(new entity::TankFireSystem);
    auto move_system = entity::System::Ptr(new entity::ObjectMoveSystem);
    auto physics_system = entity::System::Ptr(new entity::PhysicsSystem);
    auto object_checkout = entity::System::Ptr(new entity::ObjectCheckOutSystem);
    auto object_clean = entity::System::Ptr(new entity::ObjectCleanSystem);
    _sys = entity::System::Ptr(new entity::SequenceSystem({fire_system, move_system, physics_system, object_checkout, object_clean}));
}

void SurvivalView::onUpdate(float delta) {
    _c.delta(delta);
    _c.dispatcher.update();
    _sys->update(_c);
}

void SurvivalView::onDraw(SDL_Renderer* renderer) {
    _c.physics()->DebugDraw();
}

void SurvivalView::onButtonDown(int key) {
    _c.dispatcher.trigger(event::GamepadDown{key});
}

void SurvivalView::onButtonUp(int key) {
    _c.dispatcher.trigger(event::GamepadUp{key});
}

void SurvivalView::onJoyAxisMotion(JOYIDX joy_id, int x, int y) {
    _c.dispatcher.trigger(event::GamepadAxisMotion{joy_id, x, y});
}
