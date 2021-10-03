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
    template<typename T, typename... Args>
    static Ptr New(Args const&... args) {
        return Ptr(new T(args...));
    }
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
    bool has(std::string const& name) const;
    void clear();
    bool empty() const;
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

template<typename T>
class CallBackT : public Action {
public:
    typedef std::function<void(T const&)> CallFunc;
public:
    CallBackT(T const& target, CallFunc const& cf):_target(target), _func(cf) {}
    State Step(float dt) override {
        _func(_target);
        return FINISH;
    }
private:
    T _target;
    CallFunc _func;
};

class CallBackSender : public CallBackT<Widget*> {
public:
    CallBackSender(Widget* target, CallFunc const& cf):CallBackT<Widget*>(target, cf) {}
};

class CallBackDelta : public Action {
public:
    typedef std::function<void(float)> CallFunc;
public:
    CallBackDelta(CallFunc const& cf);
    State Step(float delta) override;
private:
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

class ProgressAction : public Action {
public:
    typedef std::function<void(float progress)> Callback;
    ProgressAction(Callback const& cb, float duration);
protected:
    State Step(float dt) override;
private:
    float _ticks;
    float _duration;
    Callback _callback;
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
    Blink(Widget* target, int times, float duration);
    State Step(float delta) override;
    void Reset() override;
private:
    bool _visible;
    float _timer;
    float _timer_max;
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

template <class T>
class KeepAlive : public Action {
public:
    typedef std::shared_ptr<T> TargetPtr;
    KeepAlive(TargetPtr const& target):_target(target) {}
protected:
    State Step(float delta) override {
        return FINISH;
    }
private:
    TargetPtr _target;
};

#endif //SDL2_UI_ACTION_H
