//
// Created by baifeng on 2021/11/13.
//

#include "world.h"
#include "factory.h"
#include "systems.h"
#include "components.h"
#include "object_view.h"

SurvivalView::SurvivalView() {
    _c.rootView = this;

    auto tankView = New<survival::TankView>();
    tankView->setPosition(_size.x*0.5f, _size.y*0.5f);
    addChild(tankView);

    auto _tankView = tankView->to<survival::TankView>();
    auto tank = factory::create_tank(_c.world, true);
    auto& skin = _c.world.get<component::skin>(tank);
    skin.container = _tankView;
    skin.body = _tankView->body();
    skin.weapon = _tankView->weapon();

    auto input = entity::System::Ptr(new entity::InputSystem(_c));
    _sys = entity::System::Ptr(new entity::SequenceSystem({input}));
}

void SurvivalView::onUpdate(float delta) {
    _c.delta = delta;
    _c.dispatcher.update();
    _sys->update(_c);
}

void SurvivalView::onButtonDown(int key) {
    _c.dispatcher.trigger(event::GamepadDown{key});
}

void SurvivalView::onButtonUp(int key) {
    _c.dispatcher.trigger(event::GamepadUp{key});
}
