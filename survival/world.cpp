//
// Created by baifeng on 2021/11/13.
//

#include "world.h"
#include "factory.h"
#include "systems.h"
#include "workers.h"
#include "components.h"
#include "object_view.h"
#include "common/loadres.h"

SurvivalView::SurvivalView() {
    _c.rootView = this;

    entity::tank::create(_c, entity::tank::PLAYER, size() * 0.5f);

    _worker.push_back(entity::Worker::Ptr(new entity::PlayerInputWorker(_c)));

    auto fire_system = entity::System::Ptr(new entity::TankFireSystem);
    auto move_system = entity::System::Ptr(new entity::ObjectMoveSystem);
    auto object_checkout = entity::System::Ptr(new entity::ObjectCheckOutSystem);
    auto object_clean = entity::System::Ptr(new entity::ObjectCleanSystem);
    _sys = entity::System::Ptr(new entity::SequenceSystem({fire_system, move_system, object_checkout, object_clean}));
}

void SurvivalView::onUpdate(float delta) {
    _c.delta(delta);
    _c.dispatcher.update();
    _sys->update(_c);
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
