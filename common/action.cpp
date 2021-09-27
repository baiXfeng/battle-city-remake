//
// Created by baifeng on 2021/9/25.
//

#include "action.h"
#include "view.h"
#include "game.h"

void TestAction() {

}

typedef Action::State State;

static int _actionCount = 0;
static int _actionAllocCount = 0;
std::string const _actionTitle = "action-";

//=====================================================================================

Action::Action():_name(_actionTitle+std::to_string(++_actionCount)) {
    ++_actionAllocCount;
}

Action::~Action() {
    --_actionAllocCount;
    //printf("action size = %d\n", _actionAllocCount);
}

void Action::setName(std::string const& name) {
    _name = name;
}

std::string const& Action::name() const {
    return _name;
}

//=====================================================================================

State EmptyAction::Step(float dt) {
    return FINISH;
}

//=====================================================================================

void ActionExecuter::update(float dt) {
    auto list = _actions;
    for (auto& action : list) {
        if (action->Step(dt) == Action::FINISH) {
            this->remove(action);
        }
    }
}

void ActionExecuter::add(Action::Ptr const& action) {
    _actions.push_back(action);
}

void ActionExecuter::remove(std::string const& name) {
    for (auto iter = _actions.begin(); iter != _actions.end(); iter++) {
        if (iter->get()->name() == name) {
            _actions.erase(iter);
            return;
        }
    }
}

void ActionExecuter::remove(Action::Ptr const& action) {
    for (auto iter = _actions.begin(); iter != _actions.end(); iter++) {
        if (iter->get() == action.get()) {
            _actions.erase(iter);
            return;
        }
    }
}

void ActionExecuter::clear() {
    _actions.clear();
}

//=====================================================================================

CallBackVoid::CallBackVoid(CallFunc const& cf):_func(cf) {

}

State CallBackVoid::Step(float dt) {
    _func();
    return FINISH;
}

CallBackSender::CallBackSender(Widget* target, CallFunc const& cf):_target(target), _func(cf) {

}

State CallBackSender::Step(float dt) {
    _func(_target);
    return FINISH;
}

//=====================================================================================

Delay::Delay(float duration):_ticks(0.0f), _duration(duration) {

}

State Delay::Step(float dt) {
    _ticks += dt;
    return _ticks >= _duration ? FINISH : RUNNING;
}

void Delay::Reset() {
    _ticks = 0.0f;
}

//=====================================================================================

Repeat::Repeat(Action::Ptr const& action, int repeatCount):_action(action), _repeatCount(repeatCount), _currCount(0) {

}

State Repeat::Step(float dt) {
    if (_repeatCount == 0) {
        // 永久循环
        if (_action->Step(dt) == FINISH) {
            _action->Reset();
        }
    } else if (_repeatCount >= 1) {
        if (_action->Step(dt) == FINISH) {
            if (++_currCount >= _repeatCount) {
                return FINISH;
            }
        }
    } else {
        return FINISH;
    }
    return RUNNING;
}

void Repeat::Reset() {
    _action->Reset();
    _currCount = 0;
}

//=====================================================================================

Sequence::Sequence(Actions const& actions):_actions(actions), _index(0) {

}

State Sequence::Step(float dt) {
    auto& action = _actions[_index];
    if (action->Step(dt) == FINISH) {
        ++_index;
    }
    return _index >= _actions.size() ? FINISH : RUNNING;
}

void Sequence::Reset() {
    for (auto& action : _actions) {
        action->Reset();
    }
    _index = 0;
}

//=====================================================================================

WidgetAction::WidgetAction(Widget* target, float duration):_target(target), _duration(duration), _ticks(0.0f) {

}

State WidgetAction::Step(float dt) {
    _ticks += dt;
    float progress = _ticks / _duration;
    if (progress >= 1.0f) {
        this->onFinish();
        return FINISH;
    }
    float delta = dt / ((1 - progress) * _duration);
    this->onStep(progress, delta);
    return RUNNING;
}

void WidgetAction::Reset() {
    _ticks = 0.0f;
}

//=====================================================================================

ScaleTo::ScaleTo(Widget* target, Vector2f const& scale, float duration):WidgetAction(target, duration), _scale(scale) {

}

void ScaleTo::onFinish() {
    _target->setScale(_scale);
}

void ScaleTo::onStep(float progress, float delta) {
    auto distance = (_scale - _target->scale()) * Vector2f{delta, delta};
    _target->setScale(_target->scale() + distance);
}

//=====================================================================================

ScaleBy::ScaleBy(Widget* target, Vector2f const& scale, float duration):WidgetAction(target, duration), _scale(scale), _distance(scale) {

}

void ScaleBy::onFinish() {
    _target->setScale(_target->scale()+_distance);
}

void ScaleBy::onStep(float progress, float delta) {
    auto scale = _scale * Vector2f{1 - progress, 1 - progress};
    auto distance = scale * Vector2f{delta, delta};
    _target->setScale(_target->scale() + distance);
    _distance -= distance;
}

void ScaleBy::Reset() {
    WidgetAction::Reset();
    _distance = _scale;
}

//=====================================================================================

MoveTo::MoveTo(Widget* target, Vector2f const& position, float duration):WidgetAction(target, duration), _position(position) {
}

void MoveTo::onFinish() {
    _target->setPosition(_position);
}

void MoveTo::onStep(float progress, float delta) {
    auto distance = (_position - _target->position()) * Vector2f{delta, delta};
    _target->setPosition(_target->position() + distance);
}

//=====================================================================================

MoveBy::MoveBy(Widget* target, Vector2f const& position, float duration):WidgetAction(target, duration), _position(position), _distance(position) {
}

void MoveBy::onFinish() {
    _target->setPosition(_target->position()+_distance);
}

void MoveBy::onStep(float progress, float delta) {
    auto position = _position * Vector2f{1 - progress, 1 - progress};
    auto distance = position * Vector2f{delta, delta};
    _target->setPosition(_target->position() + distance);
    _distance -= distance;
}

void MoveBy::Reset() {
    WidgetAction::Reset();
    _distance = _position;
}

//=====================================================================================

Blink::Blink(Widget* target, float duration):_target(target), _duration(duration), _visible(target->visible()), _ticks(0.0f), _timer(0) {

}

State Blink::Step(float delta) {
    _ticks += delta;
    if (_ticks >= _duration) {
        return FINISH;
    }
    _timer += delta * 1000;
    if (_timer >= 16 * 8) {
        _target->setVisible(!_target->visible());
        _timer = 0;
    }
    return RUNNING;
}

void Blink::Reset() {
    _target->setVisible(_visible);
    _ticks = 0.0f;
    _timer = 0;
}

//=====================================================================================

IPushSceneAction::IPushSceneAction(bool replace):_replace(replace) {

}

State IPushSceneAction::Step(float delta) {
    auto scene = this->create();
    if (_replace) {
        _game.screen().replace(scene);
    } else {
        _game.screen().push(scene);
    }
    return FINISH;
}