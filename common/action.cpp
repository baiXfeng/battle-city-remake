//
// Created by baifeng on 2021/9/25.
//

#include "action.h"
#include "widget.h"
#include "mutex.h"
#include <algorithm>

mge_begin

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

ActionExecuter::ActionExecuter():_pause(false) {

}

void ActionExecuter::pause(bool v) {
    _pause = v;
}

void ActionExecuter::update(float dt) {
    if (_pause) {
        return;
    }
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
    auto iter = std::find(_actions.begin(), _actions.end(), action);
    if (iter != _actions.end()) {
        _actions.erase(iter);
    }
}

bool ActionExecuter::has(std::string const& name) const {
    for (auto iter = _actions.begin(); iter != _actions.end(); iter++) {
        if (iter->get()->name() == name) {
            return true;
        }
    }
    return false;
}

void ActionExecuter::clear() {
    _actions.clear();
}

bool ActionExecuter::empty() const {
    return _actions.empty();
}

//=====================================================================================

SafeActionExecuter::SafeActionExecuter():_mutex(std::make_shared<Mutex>()) {

}

void SafeActionExecuter::add(Action::Ptr const& action) {
    Lock lock(*_mutex.get());
    ActionExecuter::add(action);
}

void SafeActionExecuter::remove(std::string const& name) {
    Lock lock(*_mutex.get());
    ActionExecuter::remove(name);
}

void SafeActionExecuter::remove(Action::Ptr const& action) {
    Lock lock(*_mutex.get());
    ActionExecuter::remove(action);
}

bool SafeActionExecuter::has(std::string const& name) const {
    Lock lock(*_mutex.get());
    return ActionExecuter::has(name);
}

void SafeActionExecuter::clear() {
    Lock lock(*_mutex.get());
    ActionExecuter::clear();
}

//=====================================================================================

CallBackVoid::CallBackVoid(CallFunc const& cf):_func(cf) {

}

State CallBackVoid::Step(float dt) {
    _func();
    return FINISH;
}

CallBackDelta::CallBackDelta(CallFunc const& cf):_func(cf) {

}

State CallBackDelta::Step(float dt) {
    _func(dt);
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
            } else {
                _action->Reset();
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

ProgressAction::ProgressAction(Callback const& cb, float duration):_callback(cb), _duration(duration), _ticks(0.0f) {

}

State ProgressAction::Step(float dt) {
    if ((_ticks += dt) >= _duration) {
        _callback(1.0f);
        return FINISH;
    }
    auto value = _ticks / _duration;
    _callback(value >= 1.0f ? 1.0f : value);
    return RUNNING;
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

Blink::Blink(Widget* target, int times, float duration):
_target(target),
_duration(duration),
_visible(target->visible()),
_ticks(0.0f),
_timer(0),
_timer_max(duration / (times * 2)) {

}

State Blink::Step(float delta) {
    auto state = RUNNING;
    if ((_ticks += delta) >= _duration) {
        state = FINISH;
    }
    if ((_timer += delta) >= _timer_max) {
        _target->setVisible(!_target->visible());
        _timer -= _timer_max;
    }
    return state;
}

void Blink::Reset() {
    _target->setVisible(_visible);
    _ticks = 0.0f;
    _timer = 0.0f;
}

mge_end
