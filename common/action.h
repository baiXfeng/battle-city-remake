//
// Created by baifeng on 2021/9/25.
//

#ifndef SDL2_UI_ACTION_H
#define SDL2_UI_ACTION_H

#include <vector>
#include <memory>
#include <string>
#include <list>
#include <functional>
#include "types.h"

void TestAction();

class Action {
public:
    enum State {
        RUNNING = 0,
        FINISH
    };
    typedef std::shared_ptr<Action> Ptr;
public:
    Action();
    virtual ~Action();
public:
    virtual State Step(float dt) = 0;
    virtual void Reset() {}
public:
    void setName(std::string const& name);
    std::string const& name() const;
private:
    std::string _name;
};

class EmptyAction : public Action {
private:
    State Step(float dt) override;
};

class Widget;
class ActionExecuter {
public:
    typedef std::list<Action::Ptr> Actions;
public:
    void update(float dt);
    void add(Action::Ptr const& action);
    void remove(std::string const& name);
    void remove(Action::Ptr const& action);
    void clear();
protected:
    Actions _actions;
};

class CallBackVoid : public Action {
public:
    typedef std::function<void()> CallFunc;
public:
    CallBackVoid(CallFunc const& cf);
    State Step(float dt) override;
private:
    CallFunc _func;
};

class CallBackSender : public Action {
public:
    typedef std::function<void(Widget*)> CallFunc;
public:
    CallBackSender(Widget* target, CallFunc const& cf);
    State Step(float dt) override;
private:
    Widget* _target;
    CallFunc _func;
};

class Delay : public Action {
public:
    Delay(float duration);
    State Step(float dt) override;
    void Reset() override;
private:
    float _ticks;
    float _duration;
};

class Repeat : public Action {
public:
    Repeat(Action::Ptr const& action, int repeatCount = 0);
    State Step(float dt) override;
    void Reset() override;
private:
    int _repeatCount;
    int _currCount;
    Action::Ptr _action;
};

class Sequence : public Action {
public:
    typedef std::vector<Action::Ptr> Actions;
public:
    Sequence(Actions const& actions);
    State Step(float dt) override;
    void Reset() override;
private:
    int _index;
    Actions _actions;
};

class WidgetAction : public Action {
public:
    WidgetAction(Widget* target, float duration);
    State Step(float dt) override;
    void Reset() override;
protected:
    virtual void onFinish() {}
    virtual void onStep(float progress, float delta) {}
protected:
    Widget* _target;
    float _duration;
    float _ticks;
};

class ScaleTo : public WidgetAction {
public:
    ScaleTo(Widget* target, Vector2f const& scale, float duration);
private:
    void onFinish() override;
    void onStep(float progress, float delta) override;
private:
    Vector2f _scale;
};

class ScaleBy : public WidgetAction {
public:
    ScaleBy(Widget* target, Vector2f const& scale, float duration);
private:
    void onFinish() override;
    void onStep(float progress, float delta) override;
    void Reset() override;
private:
    Vector2f _scale;
    Vector2f _distance;
};

class MoveTo : public WidgetAction {
public:
    MoveTo(Widget* target, Vector2f const& position, float duration);
private:
    void onFinish() override;
    void onStep(float progress, float delta) override;
private:
    Vector2f _position;
};

class MoveBy : public WidgetAction {
public:
    MoveBy(Widget* target, Vector2f const& position, float duration);
private:
    void onFinish() override;
    void onStep(float progress, float delta) override;
    void Reset() override;
private:
    Vector2f _position;
    Vector2f _distance;
};

class Blink : public Action {
public:
    Blink(Widget* target, float duration);
    State Step(float delta) override;
    void Reset() override;
private:
    bool _visible;
    int _timer;
    Widget* _target;
    float _duration;
    float _ticks;
};

class IPushSceneAction : public Action {
public:
    typedef std::shared_ptr<Widget> WidgetPtr;
public:
    IPushSceneAction(bool replace = false);
protected:
    virtual WidgetPtr create() = 0;
    State Step(float delta) override;
protected:
    bool _replace;
};

template<class T>
class PushSceneAction : public IPushSceneAction {
public:
    PushSceneAction(bool replace):IPushSceneAction(replace) {}
private:
    WidgetPtr create() override {
        return WidgetPtr(new T);
    }
};

class PopSceneAction : public Action {
private:
    State Step(float delta) override;
};

#endif //SDL2_UI_ACTION_H
